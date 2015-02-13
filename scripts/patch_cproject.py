from lxml import etree
import argparse
import subprocess

parser = argparse.ArgumentParser()
parser.add_argument("-p", "--pythonpath", help="Python path")
parser.add_argument("-s", "--scriptpath", help="Path to directory containing generate_targetconfig.py", required="true")
parser.add_argument("-c", "--cproject", help="Eclipse .cproject file", required="true")
parser.add_argument("-d", "--device", help="Target device", required="true")
parser.add_argument("-f", "--flashdriver", help="Flash driver file", required="true")
parser.add_argument("-r", "--resetscript", help="Reset script", required="true")
args = parser.parse_args()

pythonPath = 'python' if (args.pythonpath == None) else args.pythonpath
scriptDir = args.scriptpath
cprojectfile = args.cproject
device = args.device
flashDriver = args.flashdriver
resetScript = args.resetscript

p = subprocess.Popen("%s %s/generate_targetconfig.py -d %s -f %s -r %s" % (pythonPath, scriptDir, device, flashDriver, resetScript), shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
out, err = p.communicate()
targetConfigStr = out

doc = etree.parse(cprojectfile)
cproject_e = doc.getroot()

dataToWrite = False

########################
# Target Configuration #
########################
# Check if data exists
crtConfig_el = doc.xpath("/cproject/storageModule[@moduleId='com.crt.config']/projectStorage")
if len(crtConfig_el) > 0 and (len(crtConfig_el[0].text) > 0):
    print("Target Configuration data already exists.")
# Add data
else:
    if len(doc.xpath("/cproject/storageModule[@moduleId='com.crt.config']")) > 0:
        storageModule = doc.xpath("/cproject/storageModule[@moduleId='com.crt.config']")[0]
    else:
        storageModule = etree.SubElement(cproject_e, 'storageModule', attrib={'moduleId':'com.crt.config'})
    if len(doc.xpath("/cproject/storageModule[@moduleId='com.crt.config']/projectStorage")) > 0:
        projectStorage = doc.xpath("/cproject/storageModule[@moduleId='com.crt.config']/projectStorage")[0]
    else:
        projectStorage = etree.SubElement(storageModule, 'projectStorage')
    projectStorage.text = targetConfigStr.decode('utf-8').replace('\n', '').replace('\r', '')
    print("Target Configuration data added.")
    dataToWrite = True

##################
# Binary Parsers #
##################
# Check if Binary Parser exists: ELF
elf_el = doc.xpath("///extension[@id='org.eclipse.cdt.core.ELF']")
if len(elf_el) > 0:
    print("Binary Parser already exists: ELF")
# Add Binary Parser: ELF
else:
    coreSettingsList = doc.xpath("/cproject/storageModule[@moduleId='org.eclipse.cdt.core.settings']/cconfiguration/storageModule[@name='Configuration']")
    if len(coreSettingsList) > 0:
        coreSettings = coreSettingsList[0]
        if len(coreSettings.xpath("extensions")) > 0:
            extensions = coreSettings.xpath("extensions")[0]
        else:
            extensions = etree.SubElement(coreSettings, 'extensions')
        etree.SubElement(extensions, 'extension', attrib={'id':'org.eclipse.cdt.core.ELF', 'point':'org.eclipse.cdt.core.BinaryParser'})
        print("Binary Parser set: ELF")
        dataToWrite = True
    else:
        print("WARNING: Could not set Binary Parser: ELF")

# Check if Binary Parser exists: GNU ELF
elf_el = doc.xpath("///extension[@id='org.eclipse.cdt.core.GNU_ELF']")
if len(elf_el) > 0:
    print("Binary Parser already exists: GNU ELF")
# Add Binary Parser: ELF
else:
    coreSettingsList = doc.xpath("/cproject/storageModule[@moduleId='org.eclipse.cdt.core.settings']/cconfiguration/storageModule[@name='Configuration']")
    if len(coreSettingsList) > 0:
        coreSettings = coreSettingsList[0]
        if len(coreSettings.xpath("extensions")) > 0:
            extensions = coreSettings.xpath("extensions")[0]
        else:
            extensions = etree.SubElement(coreSettings, 'extensions')
        etree.SubElement(extensions, 'extension', attrib={'id':'org.eclipse.cdt.core.GNU_ELF', 'point':'org.eclipse.cdt.core.BinaryParser'})
        print("Binary Parser set: GNU ELF")
        dataToWrite = True
    else:
        print("WARNING: Could not set Binary Parser: GNU ELF")

if dataToWrite:
    doc.write(args.cproject, pretty_print=True, xml_declaration=True, encoding=doc.docinfo.encoding, standalone=doc.docinfo.standalone)