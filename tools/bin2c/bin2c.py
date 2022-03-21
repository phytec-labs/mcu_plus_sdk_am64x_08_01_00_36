import sys
import os
import socket 

if (len(sys.argv) != 4 ) :
    print("USAGE: <path/to/python3>/python bin2c.py <binary file name> <c file name> <array name>");
    exit(0);
if ( os.path.isfile(sys.argv[1]) is False) :
    print("ERROR: Input file %s not found !!!" % (sys.argv[1]));
    exit(0);

header = """
/*  Copyright (C) 2018 Texas Instruments Incorporated
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*    Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the
*    distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  \"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/

#define %s_SIZE_IN_BYTES (%dU)

#define %s { \\
"""

infile = open(sys.argv[1], "rb");
infilesize = os.stat(sys.argv[1]).st_size;
outfile = open(sys.argv[2], "w");

# write header
outfile.write(header % (sys.argv[3].upper(), infilesize, sys.argv[3].upper()) );
outfile.write( "    ");

count = 0;
while True:
    # read upto 4 bytes
    byte = infile.read(1);
    if byte :
        count = count + 1;
        # convert 32b word to hex string, then convert to integer and then convert to little endian
        # and then write to file as a C hex string
        outfile.write( "0x%sU, " % ( byte.hex() ) );
        # break to new line after 16 bytes
        if( count == 16 ) :
            outfile.write( " \\\n    ");
            count = 0;
    else :
        break;

outfile.write( " \\");
outfile.write("\n} /* %d bytes */" % (infilesize) );
outfile.write("\n");

outfile.close();
infile.close();