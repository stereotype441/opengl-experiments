import sys
import os.path

ROOT_PATH = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
sys.path.insert(1, os.path.join(ROOT_PATH, 'build_tools'))

import lwo_parser
import math_3d

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

def output_pols(layer, typ, pols, header, cpp):
    dim = max(len(poly) for flags, poly in pols)
    massaged_data = [
        poly + (-1,)*(dim - len(poly)) for flags, poly in pols]
    pols_array_name = 'layer_{0}_pols_{1}'.format(layer, typ)
    num_polys = len(massaged_data)
    header.append(
        "const int {0}_size = {1};".format(pols_array_name, num_polys))
    header.append("const int {0}_dim = {1};".format(pols_array_name, dim))
    header.append(
        "extern int {0}[{1}][{2}];".format(
            pols_array_name, num_polys, dim))
    cpp.append(
        "int {0}[{1}][{2}] = {{".format(
            pols_array_name, num_polys, dim))
    for poly in massaged_data:
        cpp.append("    {{ {0} }},".format(', '.join(str(p) for p in poly)))
    cpp.append("};")
    header.append(
        "extern int {0}_sizes[{1}];".format(pols_array_name, num_polys))
    cpp.append(
        "int {0}_sizes[{1}] = {{".format(pols_array_name, num_polys))
    for flags, poly in pols:
        cpp.append("    {0},".format(len(poly)));
    cpp.append("};")

def output_triangles(layer, typ, tris, header, cpp):
    tris_array_name = 'layer_{0}_tris_{1}'.format(layer, typ)
    tris = tuple(tris)
    num_tris = len(tris)
    header.append(
        "const int {0}_size = {1};".format(tris_array_name, num_tris))
    header.append("extern int {0}[{1}][3];".format(tris_array_name, num_tris))
    cpp.append("int {0}[{1}][3] = {{".format(tris_array_name, num_tris))
    for tri in tris:
        cpp.append("    {{ {0} }},".format(', '.join(str(p) for p in tri)))
    cpp.append("};")

header = []
cpp = ['#include "generated.h"']
pnts = None

for typ, detail in model[0][1]['data']:
    if typ == 'LAYR':
        layer = detail['number']
    elif typ == 'PNTS':
        pnts = detail
        output_points(layer, detail, header, cpp)
    elif typ == 'POLS':
        if detail['type'] != 'BONE':
            polygons = lwo_parser.pols_data_to_polygons(detail)
            for poly in polygons:
                math_3d.check_polygon(pnts, poly)
            output_triangles(
                layer, detail['type'],
                math_3d.polygons_to_triangles(pnts, polygons), header, cpp)
        output_pols(layer, detail['type'], detail['polygons'], header, cpp)

with open('generated.h', 'w') as f:
    f.write('\n'.join(header))
with open('generated.cpp', 'w') as f:
    f.write('\n'.join(cpp))
