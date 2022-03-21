import os
import sys
import getopt
import time
import shlex

try:
    import serial
    from tqdm import tqdm
    from xmodem import XMODEM, XMODEM1k
except ImportError:
    print('[ERROR] Dependant modules not installed, use below pip command to install them. MAKE sure proxy for pip is setup if needed.')
    print('');
    print('python -m pip install pyserial tqdm xmodem --proxy={http://your proxy server:port or leave blank if no proxy}');
    sys.exit();

class LineCfg():
    def __init__(self, line=None, filename=None, optype=None, offset=None, erase_size=None, flashwriter=None, cfg_src="cfg"):
        self.line = line
        self.filename = filename
        self.optype = optype
        self.offset = offset
        self.flashwriter = flashwriter
        self.erase_size = erase_size
        self.found_flashwriter_cmd = False
        self.ops_invalid = False
        self.cfg_src = cfg_src
        self.exit_now = False

    # Takes a string of comma separated key=value pairs and parses into a dictionary
    def parse_to_dict(self, config_string):
        config_dict = dict()
        splitter = shlex.shlex(config_string, posix=True)
        splitter.commenters="#"
        splitter.whitespace = ' '
        splitter.whitespace_split = True

        for key_value_pair in splitter:
            kv = key_value_pair.strip()
            if not kv:
                continue
            kv_t = kv.split('=', 1)
            if(len(kv_t)==1):
                #error, no value
                pass
            else:
                config_dict[kv_t[0]] = kv_t[1]

        return config_dict

    def validate(self):
        status = 0
        optypes = ["flash", "flashverify", "erase", "flash-xip", "flashverify-xip", "flash-phy-tuning-data", "flash-emmc", "flashverify-emmc"]
        # Check if called by config file parser or direct command line
        if(self.line!=None and self.cfg_src=="cfg"):
            # Called from config_file
            config_dict = self.parse_to_dict(self.line)

            if not config_dict:
                status = "invalid_line"
            else:
                # check for flashwriter
                if "--flash-writer" not in config_dict.keys():
                    # it is okay not to have a flashwriter, in this case
                    # user is expected to have flashed the flashwriter upfront
                    # Check for other arguments
                    # check if file is present
                    if "--operation" not in config_dict.keys():
                        status = "[ERROR] No file operation specified !!!"
                        return status
                    else:
                        self.optype = config_dict["--operation"]

                    if(self.optype == "flash" or self.optype == "flashverify" or self.optype == "erase" or self.optype == "flash-emmc" or\
                        self.optype == "flashverify-emmc"):
                        if "--flash-offset" not in config_dict.keys():
                            status = "[ERROR] Operation selected was {}, but no offset provided !!!".format(self.optype)
                            return status
                        else:
                            self.offset = config_dict["--flash-offset"]

                    if(self.optype == "flash" or self.optype == "flashverify" or self.optype == "flash-xip" or self.optype == "flashverify-xip" or\
                        self.optype == "flash-emmc" or self.optype == "flashverify-emmc"):
                        if "--file" not in config_dict.keys():
                            status = "[ERROR] Operation selected was {}, but no filename provided !!!".format(self.optype)
                            return status
                        else:
                            self.filename = config_dict["--file"]

                    if(self.optype == "erase"):
                        if "--erase-size" not in config_dict.keys():
                            status = "[ERROR] Operation selected was {}, but no erase size provided !!!".format(self.optype)
                            return status
                        else:
                            self.erase_size = config_dict["--erase-size"]

                    #No errors in parsing, now validate params to the extent possible

                    if(self.optype not in ("erase", "flash-phy-tuning-data")):
                        try:
                            f = open(self.filename)
                        except FileNotFoundError:
                            status = "[ERROR] File not found !!!"
                            return status

                    if(self.optype not in optypes):
                        status = "[ERROR] Invalid File Operation type !!!"
                        return status

                else:
                    # we have a flash writer argument, other arguments are moot
                    self.flashwriter = config_dict["--flash-writer"]

                    try:
                        f = open(self.flashwriter)
                    except FileNotFoundError:
                        status = "[ERROR] Flashwriter file not found !!!"
                        return status

        elif(self.line==None and self.cfg_src=="cmd"):
            # Called from main, with cmd line arguments
            # We have the filename, optype etc already filled
            if(self.flashwriter != None):
                self.found_flashwriter_cmd = True
            else:
                self.found_flashwriter_cmd = False

            if(self.optype == None):
                # mandatory arg. If no flashwriter, caller should exit now
                self.ops_invalid = True
                self.exit_now = not self.found_flashwriter_cmd
                if(self.found_flashwriter_cmd == False):
                    status = "[ERROR] No operation type mentioned !!!"
                    return status
                else:
                    pass
            else:
                if((self.optype == "flash" or self.optype == "flashverify" or self.optype == "erase" or self.optype == "flashverify-emmc") and (self.offset == None)):
                    self.ops_invalid = True
                    self.exit_now = not self.found_flashwriter_cmd
                    # flash/verify flash/erase, but no offset given. exit with help if no flashwriter
                    if(self.found_flashwriter_cmd == False):
                        status = "[ERROR] Operation selected was {}, but no offset was provided !!!".format(self.optype)
                        return status
                    else:
                        pass

                if((self.optype == "flash" or self.optype == "flashverify" or self.optype == "flash-xip" or self.optype == "flashverify-xip" or self.optype == "flashverify-emmc") and (self.filename == None)):
                    self.ops_invalid = True
                    self.exit_now = not self.found_flashwriter_cmd
                    # flash/verify flash/erase, but no filename given. exit with help if no flashwriter
                    if(self.found_flashwriter_cmd == False):
                        status = "[ERROR] Operation selected was {}, but no filename was provided !!!".format(self.optype)
                        return status
                    else:
                        pass

                if(self.optype == "erase" and self.erase_size == None):
                    self.ops_invalid = True
                    self.exit_now = not self.found_flashwriter_cmd
                    # erase command found, but no erase size given. exit with help if no flashwriter
                    if(self.found_flashwriter_cmd == False):
                        status = "[ERROR] Operation selected was {}, but no erase size was provided !!!".format(self.optype)
                        return status
                    else:
                        pass

                if(self.optype not in optypes):
                    self.ops_invalid = True
                    self.exit_now = not self.found_flashwriter_cmd
                    # erase command found, but no erase size given. exit with help if no flashwriter
                    if(self.found_flashwriter_cmd == False):
                        status = "[ERROR] Invalid File Operation type !!!"
                        return status
                    else:
                        pass
                if(self.optype not in ("erase", "flash-phy-tuning-data")):
                    try:
                        f = open(self.filename)
                    except FileNotFoundError:
                        self.ops_invalid = True
                        self.exit_now = not self.found_flashwriter_cmd
                        # erase command found, but no erase size given. exit with help if no flashwriter
                        if(self.found_flashwriter_cmd == False):
                            status = "[ERROR] File not found !!!"
                            return status
                        else:
                            pass
        else:
            # Should not hit this
            status = -1
        return status

class FileCfg():
    def __init__(self, filename=None):
        self.filename = filename
        self.lines = list()
        self.cfgs = list()
        self.errors = list()
        self.flash_writer_index = None

    def parse(self):
        f = open(self.filename, "r")
        lines = f.readlines()
        f.close()
        parse_status = 0
        linecount = 0
        found_fw = False
        valid_cfg_count = 0

        for line in lines:
            linecfg = LineCfg(line=line)
            status = linecfg.validate()

            if(status != 0 and status != "invalid_line"):
                self.errors.append("[ERROR] Parsing error found on line {} of {}\n{}\n".format(linecount+1, self.filename, status))
            else:
                if(status != "invalid_line"):
                    if(linecfg.flashwriter != None):
                        if(found_fw == True):
                            # error, another instance of flashwriter found
                            flash_writer_error_msg = "[ERROR] Redefinition of flash writer !!!"
                            self.errors.append("[ERROR] Parsing error found on line {} of {}\n{}\n".format(linecount+1, self.filename, flash_writer_error_msg))
                        else:
                            found_fw = True
                            self.cfgs.append(linecfg)
                            self.lines.append(line.rstrip('\n'))
                            self.flash_writer_index = valid_cfg_count
                            valid_cfg_count += 1
                    else:
                        self.cfgs.append(linecfg)
                        self.lines.append(line.rstrip('\n'))
                        valid_cfg_count += 1
            linecount += 1

        if(len(self.errors) > 0):
            # Some commands have errors
            parse_status = "Parsing config file ... ERROR. {} error(s).\n\n".format(len(self.errors)) + "\n".join(self.errors)

        return parse_status

BOOTLOADER_UNIFLASH_BUF_SIZE                         = 1024*1024 # 1 MB This has to be a 256 KB aligned value, because flash writes will be block oriented
BOOTLOADER_UNIFLASH_HEADER_SIZE                      = 32 # 32 B

BOOTLOADER_UNIFLASH_FILE_HEADER_MAGIC_NUMBER         = bytearray([0x42, 0x4C ,0x55, 0x46]) # BLUF
BOOTLOADER_UNIFLASH_RESP_HEADER_MAGIC_NUMBER         = 0x52554C42 # BLUR

BOOTLOADER_UNIFLASH_OPTYPE_FLASH                     = bytearray([0xF0, 0x00, 0x00, 0x00])
BOOTLOADER_UNIFLASH_OPTYPE_FLASH_VERIFY              = bytearray([0xF1, 0x00, 0x00, 0x00])
BOOTLOADER_UNIFLASH_OPTYPE_FLASH_XIP                 = bytearray([0xF2, 0x00, 0x00, 0x00])
BOOTLOADER_UNIFLASH_OPTYPE_FLASH_VERIFY_XIP          = bytearray([0xF3, 0x00, 0x00, 0x00])
BOOTLOADER_UNIFLASH_OPTYPE_FLASH_TUNING_DATA         = bytearray([0xF4, 0x00, 0x00, 0x00])
BOOTLOADER_UNIFLASH_OPTYPE_FLASH_ERASE               = bytearray([0xFE, 0x00, 0x00, 0x00])
BOOTLOADER_UNIFLASH_OPTYPE_EMMC_FLASH                = bytearray([0xF5, 0x00, 0x00, 0x00])
BOOTLOADER_UNIFLASH_OPTYPE_EMMC_VERIFY               = bytearray([0xF6, 0x00, 0x00, 0x00])

BOOTLOADER_UNIFLASH_IMAGE_TYPE_MULTICORE             = bytearray([0x4D, 0x43, 0x00, 0x00]) # MC
BOOTLOADER_UNIFLASH_IMAGE_TYPE_UBOOT                 = bytearray([0x55, 0x42, 0x00, 0x00]) # UB

BOOTLOADER_UNIFLASH_STATUSCODE_SUCCESS                = 0x00000000
BOOTLOADER_UNIFLASH_STATUSCODE_MAGIC_ERROR            = 0x10000001
BOOTLOADER_UNIFLASH_STATUSCODE_OPTYPE_ERROR           = 0x20000001
BOOTLOADER_UNIFLASH_STATUSCODE_FLASH_ERROR            = 0x30000001
BOOTLOADER_UNIFLASH_STATUSCODE_FLASH_VERIFY_ERROR     = 0x40000001
BOOTLOADER_UNIFLASH_STATUSCODE_FLASH_ERASE_ERROR      = 0x50000001

optypewords = {
    "flash" : BOOTLOADER_UNIFLASH_OPTYPE_FLASH,
    "flashverify" : BOOTLOADER_UNIFLASH_OPTYPE_FLASH_VERIFY,
    "flash-xip" : BOOTLOADER_UNIFLASH_OPTYPE_FLASH_XIP,
    "flashverify-xip" : BOOTLOADER_UNIFLASH_OPTYPE_FLASH_VERIFY_XIP,
    "flash-phy-tuning-data" : BOOTLOADER_UNIFLASH_OPTYPE_FLASH_TUNING_DATA,
    "erase" : BOOTLOADER_UNIFLASH_OPTYPE_FLASH_ERASE,
    "flash-emmc":BOOTLOADER_UNIFLASH_OPTYPE_EMMC_FLASH,
    "flashverify-emmc":BOOTLOADER_UNIFLASH_OPTYPE_EMMC_VERIFY,
}

statuscodes = {
    BOOTLOADER_UNIFLASH_STATUSCODE_SUCCESS : "[STATUS] SUCCESS !!!\n",
    BOOTLOADER_UNIFLASH_STATUSCODE_MAGIC_ERROR : "[STATUS] ERROR: Incorrect magic number in file header !!!\n",
    BOOTLOADER_UNIFLASH_STATUSCODE_OPTYPE_ERROR : "[STATUS] ERROR: Invalid file operation type sent !!!\n",
    BOOTLOADER_UNIFLASH_STATUSCODE_FLASH_ERROR : "[STATUS] ERROR: Flashing failed !!!\n",
    BOOTLOADER_UNIFLASH_STATUSCODE_FLASH_VERIFY_ERROR : "[STATUS] ERROR: Flash verify failed !!!\n",
    BOOTLOADER_UNIFLASH_STATUSCODE_FLASH_ERASE_ERROR : "[STATUS] ERROR: Flash erase failed !!!\n",
}

TMP_SUFFIX = ".tmp"

usage_string = '''
USAGE: python uart_uniflash.py [OPTIONS]

-p, --serial-port=   Serial port to use for the transfer.
                     COM3, COM4 etc if on windows and /dev/ttyUSB1,
                     /dev/ttyUSB2 etc if on linux. Mandatory argument

-f, --file=          Filename to send for an operation. Not required if
                     using config mode (--cfg)

--operation=         Operation to be done on the file
                     => "flash" or "flashverify" or "erase" or "flash-xip" or "flashverify-xip" or "flash-phy-tuning-data" or "flash-emmc" or "flashverify-emmc".
                     Not required if using config mode (--cfg)

-o, --flash-offset=  Offset (in hexadecimal format starting with a 0x) at which
                     the flash/verify flash is to be done. Not required if
                     using config mode (--cfg)

--erase-size=        Size of flash to erase. Only valid when operation is "erase"

--flash-writer=      Special option. This will load the sbl_uart_uniflash binary which will
                     be booted by ROM. Other arguments are irrelevant and hence ignored
                     when --flash-writer argument is present. Not required if
                     using config mode (--cfg)

--cfg=               Configuration file which has a list of files with
                     options for flashing. This should be given as option=value pairs
                     separated by space just like in command line. Only give the long
                     options in the config file i.e --flash-offset=0x80000 and not -o 0x80000

                     Example, below can be a line in the config file, myconfig.cfg
                     --file=am64x-evm/sbl_ospi.release.tiimage --operation=flash --flash-offset=0x0

                     and then the script can be called:
                     python uart_uniflash.py -p <COM port> --cfg=myconfig.cfg
'''

bar_cursize = 0
bar_filesize = 0

mySerPort = "No Serial Port Chosen"
myBaudRate = 115200
ser = serial.Serial(timeout=10)

def open_serial_port():
    ser.baudrate = 115200
    ser.port = mySerPort
    ser.open()

def close_serial_port():
    ser.close()

def get_numword(s):
    numword = None
    if(s.startswith('0x')):
        numword = int(s, 16)
    else:
        numword = int(s, 10)
    return numword

def numstr_to_byte_array(numstr):
    numword = get_numword(numstr)
    return bytearray([numword & 0xFF, (numword>>8) & 0xFF, (numword>>16) & 0xFF, (numword>>24) & 0xFF])

# Add the file header with metadata to the file to be sent. Expects a validated linecfg
def create_temp_file(linecfg):
    if(linecfg.optype in ("erase", "flash-phy-tuning-data")):
        # No separate file required
        tempfilename = "{} command".format(linecfg.optype)
    else:
        tempfilename = linecfg.filename + TMP_SUFFIX

    f = open(tempfilename, "wb")

    rsv_byte_arr = bytearray([0xBE, 0xBA, 0xAD, 0xDE])
    # add header
    # add magic number
    f.write(BOOTLOADER_UNIFLASH_FILE_HEADER_MAGIC_NUMBER)

    f.write(optypewords[linecfg.optype])

    # add offset
    if(linecfg.optype == "flash-xip" or linecfg.optype == "flashverify-xip" or linecfg.optype == "flash-phy-tuning-data" ):
        f.write(rsv_byte_arr); # offset not needed for XIP operations, since the offsets are in the file itself
    else:
        offset_byte_arr = numstr_to_byte_array(linecfg.offset)
        f.write(offset_byte_arr)

    # add flash erase size
    if(linecfg.optype == "erase"):
        erase_size_byte_arr = numstr_to_byte_array(linecfg.erase_size)
        f.write(erase_size_byte_arr)
    else:
        f.write(rsv_byte_arr)

    # fill reserved bytes (4 reserved bytes now)
    for i in range(0, 4):
        f.write(rsv_byte_arr)

    # No original file in case of erase or phy tuning
    if(linecfg.optype not in ("erase","flash-phy-tuning-data")):
        # copy the original file to this file
        original_file = open(linecfg.filename, "rb")
        f.write(original_file.read())
        original_file.close()

    f.close()

    return tempfilename

# Parse response header sent from EVM
def parse_response_evm(filename):
    f = open(filename, "rb")
    resp_bytes = f.read(16)
    f.close()

    status = None

    resp_magic = int.from_bytes(resp_bytes[0:4], 'little')
    resp_status = int.from_bytes(resp_bytes[4:8], 'little')

    if(resp_magic == BOOTLOADER_UNIFLASH_RESP_HEADER_MAGIC_NUMBER):
        if resp_status in statuscodes.keys():
            status = statuscodes[resp_status]
        else:
            status = "[ERROR] Invalid status code in response !!!"
    else:
        status = "[ERROR] Incorrect magic number in Response Header !!!"

    return status

# Sends the file to EVM via xmodem, receives response from EVM and returns the response status
def xmodem_send_receive_file(filename, serialport, baudrate=115200, get_response=True):
    status = False;
    timetaken = 0
    try:
        global mySerPort, myBaudRate
        bar_filesize = os.path.getsize(filename);
        mySerPort = serialport
        myBaudRate = baudrate
        open_serial_port();
        stream = open(filename, 'rb')
    except FileNotFoundError:
        print('[ERROR] File [' + filename + '] not found !!!');
        sys.exit();
    except serial.serialutil.SerialException:
        print('[ERROR] Serial port [' + serialport + '] not found or not accessible !!!');
        sys.exit();

    bar_cursize = 0
    bar = tqdm(total=bar_filesize, unit="bytes", leave=False, desc="Sending {}".format(filename.replace(TMP_SUFFIX, "")))

    def getc(size, timeout=1):
        return ser.read(size) or None

    def putc(data, timeout=1):
        bar.update(len(data));
        bar.refresh()
        return ser.write(data)  # note that this ignores the timeout

    try:
        modem = XMODEM1k(getc, putc)
        tstart = time.time()
        status = modem.send(stream, quiet=True, timeout=10, retry=10)
        tstop = time.time()
        timetaken = round(tstop-tstart, 2)
    except:
        status = False;

    stream.close()

    if( status is False ) :
        print("");
        print ("[ERROR] XMODEM send failed, no response OR incorrect response from EVM OR cancelled by user,");
        print ("        Power cycle EVM and run this script again !!!");
        sys.exit();

    resp_status = 0

    # Don't do the receive if get_response is False
    if(get_response):
        respfilename = "resp.dat"
        try:
            respfile = open(respfilename, "wb")
            status = modem.recv(respfile, quiet=True, timeout=2000)
            respfile.close()
            resp_status = parse_response_evm(respfilename)
            os.remove(respfilename)
        except:
            status = None;
        if( status is None ) :
            print("");
            print ("[ERROR] XMODEM recv failed, no response OR incorrect response from EVM OR cancelled by user,");
            print ("        Power cycle EVM and run this script again !!!");
            sys.exit();

    close_serial_port()

    bar.close()

    return resp_status, timetaken

def send_file_by_parts(l_cfg, s_port):
    orig_f_name = l_cfg.filename
    orig_offset = l_cfg.offset

    f = open(orig_f_name, "rb")
    f_bytes = f.read()
    f.close()

    num_parts   = int(len(f_bytes) / BOOTLOADER_UNIFLASH_BUF_SIZE)
    remain_size = len(f_bytes) % BOOTLOADER_UNIFLASH_BUF_SIZE


    for i in range(0, num_parts):

        start = i*BOOTLOADER_UNIFLASH_BUF_SIZE
        end = start+BOOTLOADER_UNIFLASH_BUF_SIZE

        part_data = f_bytes[start:end]
        part_filename = orig_f_name + ".part{}".format(i+1)
        # make the partial file
        f = open(part_filename, "wb")
        f.write(part_data)
        f.close()

        # temporarily change this to the partial filename
        l_cfg.filename = part_filename
        l_cfg.offset = hex(get_numword(orig_offset) + i*BOOTLOADER_UNIFLASH_BUF_SIZE)

        # send the partial file normally
        tempfilename = create_temp_file(l_cfg)
        status, timetaken = xmodem_send_receive_file(tempfilename, s_port);

        # delete the temporary file
        os.remove(part_filename)
        os.remove(tempfilename)

    # Send the last part, if there were residual bytes
    if(remain_size > 0):
        start = num_parts*BOOTLOADER_UNIFLASH_BUF_SIZE
        end = -1 # Read till the end of original file

        part_data = f_bytes[start:end]
        part_filename = orig_f_name + ".part{}".format(num_parts+1)

        # make the partial file
        f = open(part_filename, "wb")
        f.write(part_data)
        f.close()

        # temporarily change this to the partial filename
        l_cfg.filename = part_filename
        l_cfg.offset = hex(get_numword(orig_offset) + num_parts*BOOTLOADER_UNIFLASH_BUF_SIZE)

        # send the partial file normally
        tempfilename = create_temp_file(l_cfg)
        status, timetaken = xmodem_send_receive_file(tempfilename, s_port);

        # delete the temporary file
        os.remove(part_filename)
        os.remove(tempfilename)

    # revert back the original filename and offset
    l_cfg.filename = orig_f_name
    l_cfg.offset = orig_offset


def main(argv):

    def help() :
        print(usage_string);

    serialport = None
    config_file = None
    cmd_flash_writer_found = False
    ops_invalid = False

    cmdlinecfg = LineCfg(cfg_src="cmd")

    try:
        opts, args = getopt.getopt(argv,"hf:p:o:",["help", "file=","serial-port=", "operation=", "image-type=", "flash-offset=", "cfg=", "flash-writer=", "erase-size="])
    except getopt.GetoptError:
        help();
        sys.exit();
    for opt, arg in opts:
        if opt in ("-h", "--help"):
            help();
            sys.exit()
        elif opt in ("-p", "--serial-port"):
            serialport = arg
        elif opt in ("--cfg"):
            config_file = arg
        elif opt in ("-f", "--file"):
            cmdlinecfg.filename = arg
        elif opt in ("-o", "--flash-offset"):
            cmdlinecfg.offset = arg
        elif opt in ("--operation"):
            cmdlinecfg.optype = arg
        elif opt in ("--flash-writer"):
            cmdlinecfg.flashwriter = arg
        elif opt in ("--erase-size"):
            cmdlinecfg.erase_size = arg

    if(serialport == None):
        # No serial port, main argument -> must have. Exit with help
        print("[ERROR] Provide a serial port with option -p {serial port}, use -h option to see detailed help !!!")
        sys.exit()

    else:
        if(config_file == None):
            status = cmdlinecfg.validate()
            if(status != 0):
                print(status)
                if(cmdlinecfg.exit_now == True):
                    sys.exit(2)

    if(config_file != None):
        # Check if file exists
        try:
            cfg_file_handle = open(config_file, "r")
        except FileNotFoundError:
            print('[ERROR] Configuration file [' + config_file + '] not found !!!');
            sys.exit()
        cfg_file_handle.close()

        print("")
        print("Parsing config file ...")
        filecfg = FileCfg(config_file)
        parse_status = filecfg.parse()

        if(parse_status != 0):
            print(parse_status)
            sys.exit()
        else:
            # No errors, can proceed to flash
            print("Parsing config file ... SUCCESS. Found {} command(s) !!!".format(len(filecfg.cfgs)))
            print("")

            if(filecfg.flash_writer_index != None):
                # Found flash writer, flash it
                cfg_flash_writer_file = filecfg.cfgs[filecfg.flash_writer_index].flashwriter
                print("Executing command {} of {} ...".format(1, len(filecfg.cfgs)))
                print("Found flash writer ... sending {}".format(cfg_flash_writer_file))
                status, timetaken = xmodem_send_receive_file(cfg_flash_writer_file, serialport, get_response=False)
                print("Sent flashwriter {} of size {} bytes in {}s.".format(cfg_flash_writer_file, os.path.getsize(cfg_flash_writer_file), timetaken))
                print("");

            # loop through cfgs and lines, skip the process for flashwriter
            for i in range(0, len(filecfg.cfgs)):
                if(i != filecfg.flash_writer_index):
                    line = filecfg.lines[i]
                    linecfg = filecfg.cfgs[i]
                    print("Executing command {} of {} ...".format(i+1, len(filecfg.cfgs)))
                    print("Command arguments : {}".format(line.rstrip('\n')))
                    # Check if the size of application image is larger than buffer size in target side.
                    f_size = 0
                    if linecfg.filename is not None:
                        f_size = os.path.getsize(linecfg.filename)
                    
                    if((f_size + BOOTLOADER_UNIFLASH_HEADER_SIZE >= BOOTLOADER_UNIFLASH_BUF_SIZE) and (linecfg.optype in ["flash", "flashverify"])):
                        # Send by parts
                        send_file_by_parts(linecfg, serialport)
                    else:
                        # Send normally
                        tempfilename = create_temp_file(linecfg)
                        status, timetaken = xmodem_send_receive_file(tempfilename, serialport, get_response=True)

                    orig_filename = linecfg.filename
                    if(linecfg.optype == "erase"):
                        print("Sent flash erase command.")
                    elif(linecfg.optype == "flash-phy-tuning-data"):
                        print("Sent flash phy tuning data in {}s.".format(timetaken))
                    else:
                        print("Sent {} of size {} bytes in {}s.".format(orig_filename, os.path.getsize(orig_filename), timetaken))
                    print(status)
                    # Delete the tempfile
                    os.remove(tempfilename)

            print("All commands from config file are executed !!!")

    else:
        # If flash writer was found, send it first
        if(cmdlinecfg.found_flashwriter_cmd == True):
            print("Found flash writer ... sending {}".format(cmdlinecfg.flashwriter))
            status, timetaken = xmodem_send_receive_file(cmdlinecfg.flashwriter, serialport, get_response=False);
            print("Sent flashwriter {} of size {} bytes in {}s.".format(cmdlinecfg.flashwriter, os.path.getsize(cmdlinecfg.flashwriter), timetaken))
            print("");

        if(cmdlinecfg.ops_invalid == False):
            # Check if the size of application image is larger than buffer size in target side.
            f_size = 0
            if cmdlinecfg.filename is not None:
                f_size = os.path.getsize(cmdlinecfg.filename)

            if((f_size + BOOTLOADER_UNIFLASH_HEADER_SIZE >= BOOTLOADER_UNIFLASH_BUF_SIZE) and (cmdlinecfg.optype in ["flash", "flashverify"])):
                # Send by parts
                send_file_by_parts(cmdlinecfg, serialport)
            else:
                # Send normally
                tempfilename = create_temp_file(cmdlinecfg)
                status, timetaken = xmodem_send_receive_file(tempfilename, serialport);
                # Delete the tempfile
                os.remove(tempfilename)

            orig_filename = cmdlinecfg.filename
            if(cmdlinecfg.optype == "erase"):
                print("Sent flash erase command.")
            elif(cmdlinecfg.optype == "flash-phy-tuning-data"):
                print("Sent flash phy tuning data in {}s.".format(timetaken))
            else:
                print("Sent {} of size {} bytes in {}s.".format(orig_filename, os.path.getsize(orig_filename), timetaken))
            print(status)

if __name__ == "__main__":
   main(sys.argv[1:])