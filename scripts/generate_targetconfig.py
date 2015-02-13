from lxml import etree
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("-d", "--device", help="Target device", required="true")
parser.add_argument("-f", "--flashdriver", help="Flash driver file", required="true")
parser.add_argument("-r", "--resetscript", help="Reset script", required="true")
args = parser.parse_args()

device = args.device
flashDriver = args.flashdriver
resetScript = args.resetscript

targetConfig = etree.Element('TargetConfig')
properties = etree.SubElement(targetConfig, 'Properties', attrib={'property_0':'None', 'property_2':flashDriver, 'property_3':'NXP', 'property_4':device, 'property_count':'5', 'version':'70200'})
infoList = etree.SubElement(targetConfig, 'infoList', attrib={'vendor':'NXP'})
info = etree.SubElement(infoList, 'info', attrib={'chip':device, 'flash_driver':flashDriver, 'match_id':'0x0', 'name':device, 'resetscript':resetScript, 'stub':'crt_emu_lpc18_43_nxp'})

tree = etree.ElementTree(targetConfig)
xmlStr = etree.tostring(tree.getroot(), xml_declaration=True, encoding='UTF-8', method='xml')

print(xmlStr.decode("utf-8"))