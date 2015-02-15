from lxml import etree
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("-d", "--device", help="Target device", required="true")
parser.add_argument("-f", "--flashdriver", help="Flash driver file", required="true")
parser.add_argument("-r", "--resetscript", help="Reset script", required="true")
args = parser.parse_args()

# Save passed arguments
device = args.device
flashDriver = args.flashdriver
resetScript = args.resetscript

# Create Target Configuration XML document
targetConfig = etree.Element('TargetConfig')
properties = etree.SubElement(targetConfig, 'Properties', attrib={'property_0':'None', 'property_2':flashDriver, 'property_3':'NXP', 'property_4':device, 'property_count':'5', 'version':'70200'})
infoList = etree.SubElement(targetConfig, 'infoList', attrib={'vendor':'NXP'})
info = etree.SubElement(infoList, 'info', attrib={'chip':device, 'flash_driver':flashDriver, 'match_id':'0x0', 'name':device, 'resetscript':resetScript, 'stub':'crt_emu_lpc18_43_nxp'})
chip = etree.SubElement(info, 'chip')
chip_name = etree.SubElement(chip, 'name')
chip_name.text = device
chip_family = etree.SubElement(chip, 'family')
chip_family.text = 'LPC43xx'
chip_vendor = etree.SubElement(chip, 'vendor')
chip_vendor.text = 'NXP (formerly Philips)'
processor = etree.SubElement(info, 'processor')
processor_name = etree.SubElement(processor, 'name', attrib={'gcc_name':'cortex-m4'})
processor_name.text = 'Cortex-M4'
processor_family = etree.SubElement(processor, 'family')
processor_family.text = 'Cortex-M'
link = etree.SubElement(info, 'link', attrib={'href':'nxp_lpc43xx_peripheral.xme', 'show':'embed', 'type':'simple'})

# Save XML document as string
tree = etree.ElementTree(targetConfig)
xmlStr = etree.tostring(tree.getroot(), xml_declaration=True, encoding='UTF-8', method='xml')

print(xmlStr.decode("utf-8"))