import sys
import re
import os.path

def encode_c_string(s):
    return '"{0}"'.format(re.sub('"', r'\"', s.encode('string_escape')))

shader_file_name = sys.argv[1]
shader_dir_name, shader_name = os.path.split(shader_file_name)
shader_base_name = re.sub(r'\..*', '', shader_name)

header_file_name = shader_file_name + '.g.h'
cpp_file_name = shader_file_name + '.g.cpp'

with open(shader_file_name, 'r') as f:
    shader = f.read()

header = 'extern char const *shader_{0};\n'.format(
    shader_base_name)
cpp = 'char const *shader_{0} = {1};\n'.format(
    shader_base_name, encode_c_string(shader))

with open(header_file_name, 'w') as f:
    f.write(header)
with open(cpp_file_name, 'w') as f:
    f.write(cpp)
