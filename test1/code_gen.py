import sys
import os.path

ROOT_PATH = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
sys.path.insert(1, os.path.join(ROOT_PATH, 'build_tools'))

import lwo_parser

model = lwo_parser.parse_lwo(
    lwo_parser.read_file(os.path.join(ROOT_PATH, 'models/elfegab.lwo')))

layer = None

def output_points(layer, points, header, cpp):
    points_array_name = 'layer_{0}_points'.format(layer)
    num_points = len(points)
    header.append(
        "const int num_points_in_layer_{0} = {1};".format(layer, num_points))
    header.append(
        "extern float {0}[{1}][3];".format(points_array_name, num_points))
    cpp.append("float {0}[{1}][3] = {{".format(points_array_name, num_points))
    for p in points:
        cpp.append("    {{ {0}, {1}, {2} }},".format(*p))
    cpp.append("};")

header = []
cpp = ['#include "generated.h"']

for typ, detail in model[0][1]['data']:
    if typ == 'LAYR':
        layer = detail['number']
    elif typ == 'PNTS':
        output_points(layer, detail, header, cpp)

with open('generated.h', 'w') as f:
    f.write('\n'.join(header))
with open('generated.cpp', 'w') as f:
    f.write('\n'.join(cpp))
