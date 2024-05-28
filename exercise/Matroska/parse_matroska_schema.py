# Usage: python parse_matroska_schema.py
# MUST have the schema file in the same directory (sorry I was lazy)

import os
import xml.etree.ElementTree as ET

root = ET.parse('ebml_matroska.xml').getroot()

with open('parsed_schema.txt', 'w') as out:
    out.write('''
{0x1A45DFA3, { "EBML", eleminfo::elem_type::master_t}},
{0x4286, { "EBMLVersion", eleminfo::elem_type::uinteger_t}},
{0x42F7, { "EBMLReadVersion", eleminfo::elem_type::uinteger_t}},
{0x4282, { "DocType", eleminfo::elem_type::string_t}},
{0x4287, { "DocTypeVersion", eleminfo::elem_type::uinteger_t}},
{0x4285, { "DocTypeReadVersion", eleminfo::elem_type::uinteger_t}},
{0x4281, { "DocTypeExtension", eleminfo::elem_type::master_t}},
{0x4283, { "DocTypeExtensionName", eleminfo::elem_type::string_t}},
{0x4284, { "DocTypeExtensionVersion", eleminfo::elem_type::uinteger_t}},
''')
    for child in root:
        out.write("{{{}, {{ \"{}\", eleminfo::elem_type::{}_t}}}},\n".format(child.attrib['id'], child.attrib['name'], child.attrib['type'].replace('-', '_')))
        