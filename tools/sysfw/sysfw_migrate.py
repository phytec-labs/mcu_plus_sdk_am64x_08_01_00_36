import sys
import os
import shutil
import glob

SOC                   = "am64x"
SOC_FOLDER            = "am64x_am243x"
ARCHIVE_FMT           = "tar"
SYSFW_REL_GIT_URL     = "ssh://bitbucket.itg.ti.com/sysfw/system-firmware-releases.git"
SBL_CERT_GEN          = os.path.realpath("../boot/signing/x509CertificateGen.sh")
SBL_CERT_KEY          = os.path.realpath("../boot/signing/rom_degenerateKey.pem")
SYSFW_LOAD_ADDR       = "0x44000"

sciclient_path = os.path.realpath("../../source/drivers/sciclient/")

if(os.name=="nt"):
	print("[ERROR] Script currently not supported in windows")
	exit(1)

if(not os.path.isdir(sciclient_path)):
	print("[ERROR] This script is run from the wrong path !!!")
	exit(1)

if(len(sys.argv) < 2):
	print("[ERROR] Please provide the release tag")
	print("Usage: python3 sysfw_migrate.py <release-tag>")
	exit(1)

release_tag = sys.argv[1]

def get_git_file(remote_url, branch, filepath, strip_level):
	filebasename = os.path.basename(filepath)
	os.system(f"git archive --format={ARCHIVE_FMT} --remote={remote_url} {branch} {filepath} > {filebasename}.{ARCHIVE_FMT}")
	if(os.name=="posix"):
		os.system(f"tar -xf {filebasename}.{ARCHIVE_FMT} --strip-components={strip_level}")
	os.remove(f"{filebasename}.{ARCHIVE_FMT}")

def file_replace(filename, current_pattern, changed_pattern, lineNum=None):
	lines = None
	lineCount = 0
	with open(filename, "r") as f:
		lines = f.readlines()
		lineCount = len(lines)
	if(lineNum == None):
		for i in range(0, lineCount):
			lines[i] = lines[i].replace(current_pattern, changed_pattern)
	else:
		lines[lineNum] = lines[lineNum].replace(current_pattern, changed_pattern)
	with open(filename, "w") as f:
		f.writelines(lines)

# Clone SYSFW releases
os.mkdir("sysfw-rel")
os.chdir("sysfw-rel")

# Get sysfw bin and copy to sciclient directory
print(f"Getting the SYSFW binary for {release_tag} ...")
get_git_file(SYSFW_REL_GIT_URL, release_tag, f"binaries/ti-sci-firmware-{SOC}-gp.bin", 1)
shutil.copyfile(f"ti-sci-firmware-{SOC}-gp.bin", sciclient_path+f"/soc/{SOC_FOLDER}/sysfw.bin")
print("Done !!!")

# Generate the signed sysfw binary
os.system(f"{SBL_CERT_GEN} -b {sciclient_path}/soc/{SOC_FOLDER}/sysfw.bin -o {sciclient_path}/soc/{SOC_FOLDER}/sysfw_signed.bin -c DMSC_I -l {SYSFW_LOAD_ADDR} -k {SBL_CERT_KEY}")


# Get the include files
print("Getting the include files ...")
get_git_file(SYSFW_REL_GIT_URL, release_tag, f"include/tisci", 1)
get_git_file(SYSFW_REL_GIT_URL, release_tag, f"include/{SOC}", 1)
print("Done !!!")

# Copy the tisci include files

print("Copying the include files to sciclient folder with necessary changes...")

tisci_gen_include_files = [ os.path.basename(i) for i in glob.glob("tisci/*.h") ]

for file in tisci_gen_include_files:
	shutil.copyfile("tisci/"+file, sciclient_path+"/include/tisci/"+file)
	file_replace(sciclient_path+"/include/tisci/"+file, "/* @} */", "/** @} */", -1)

tisci_pm_include_files = [ os.path.basename(i) for i in glob.glob("tisci/pm/*.h") ]

for file in tisci_pm_include_files:
	shutil.copyfile("tisci/pm/"+file, sciclient_path+"/include/tisci/pm/"+file)
	file_replace(sciclient_path+"/include/tisci/pm/"+file, "/* @} */", "/** @} */", -1)

tisci_rm_include_files = [ os.path.basename(i) for i in glob.glob("tisci/rm/*.h") ]

for file in tisci_rm_include_files:
	shutil.copyfile("tisci/rm/"+file, sciclient_path+"/include/tisci/rm/"+file)
	file_replace(sciclient_path+"/include/tisci/rm/"+file, "/* @} */", "/** @} */", -1)

tisci_security_include_files = [ os.path.basename(i) for i in glob.glob("tisci/security/*.h") ]

for file in tisci_security_include_files:
	shutil.copyfile("tisci/security/"+file, sciclient_path+"/include/tisci/security/"+file)
	file_replace(sciclient_path+"/include/tisci/security/"+file, "/* @} */", "/** @} */", -1)

tisci_soc_include_files = [ os.path.basename(i) for i in glob.glob(f"{SOC}/*.h") ]

for file in tisci_soc_include_files:
	shutil.copyfile(f"{SOC}/"+file, sciclient_path+f"/include/tisci/{SOC_FOLDER}/"+file)
	file_replace(sciclient_path+f"/include/tisci/{SOC_FOLDER}/"+file, "/* @} */", "/** @} */", -1)

print("Done !!!")

os.chdir("../")

if(os.name == "posix"):
	os.system("rm -rf sysfw-rel")














