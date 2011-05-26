# Note: LWO is described here: http://home.comcast.net/~erniew/lwsdk/docs/filefmts/lwo2.html
# And also (more poorly) here: http://gpwiki.org/index.php/LWO
# An older version is described here: http://www.martinreddy.net/gfx/3d/LWOB.txt
# It is based on IFF, which is described here: http://www.martinreddy.net/gfx/2d/IFF.txt

import struct
import json
import sys

class StringWalker(object):
    def __init__(self, s):
        self.__s = s
        self.__i = 0

    def __nonzero__(self):
        return self.__i < len(self.__s)

    def get_bytes(self, num_bytes):
        start = self.__i
        self.__i += num_bytes
        assert self.__i <= len(self.__s)
        return self.__s[start : self.__i]

    def unpack(self, fmt):
        start = self.__i
        self.__i += struct.calcsize(fmt)
        assert self.__i <= len(self.__s)
        return struct.unpack_from(fmt, self.__s, start)

    def word_align(self):
        if self.__i % 2 == 1:
            self.__i += 1

    def get_string(self):
        assert self.__i < len(self.__s)
        start = self.__i
        end = self.__s.find('\x00', self.__i)
        assert end != -1
        self.__i = end + 1
        self.word_align()
        return self.__s[start:end].decode('latin_1')

    def __repr__(self):
        return 'StringWalker({0!r})'.format(self.__s[self.__i:])

def iter_chunks(s, sub_parsers = None, size_packing = 'L'):
    if sub_parsers is None:
        sub_parsers = {}
    while s:
        chunk_id = s.get_bytes(4)
        chunk_length = s.unpack('>' + size_packing)[0]
        chunk_data = s.get_bytes(chunk_length)
        if chunk_id in sub_parsers:
            chunk_data = sub_parsers[chunk_id](StringWalker(chunk_data))
        yield chunk_id, chunk_data
        s.word_align()

def form_parser(sub_parsers):
    def parse(s):
        return {'type': s.get_bytes(4), 'data': tuple(iter_chunks(s, sub_parsers))}
    return parse

def parse_string_array(s):
    result = []
    while s:
        result.append(s.get_string())
    return tuple(result)

def parse_layr(s):
    result = {}
    result['number'] = s.unpack('>H')[0]
    result['flags'] = s.unpack('>H')[0]
    result['pivot'] = s.unpack('>fff')
    result['name'] = s.get_string()
    if s:
        result['parent'] = s.unpack('>H')[0]
    else:
        result['parent'] = 0
    return result

def parse_pnts(s):
    result = []
    while s:
        result.append(s.unpack('>fff'))
    return tuple(result)

def parse_bbox(s):
    return {'min': s.unpack('>fff'), 'max': s.unpack('>fff')}

def parse_vmap(s):
    result = {}
    result['type'] = s.get_bytes(4)
    dim = s.unpack('>H')[0]
    result['dimension'] = dim
    result['name'] = s.get_string()
    data = []
    while s:
        data.append((s.unpack('>H')[0], s.unpack('>{0}'.format('f' * dim))))
    result['data'] = tuple(data)
    return result

def parse_vx(s):
    tentative_index = s.unpack('>H')[0]
    if (tentative_index & 0xff00) == 0xff00:
        return s.unpack('>L')
    else:
        return tentative_index

def parse_pols(s):
    result = {}
    result['type'] = s.get_bytes(4)
    polygons = []
    while s:
        num_vertices_and_flags = s.unpack('>H')[0]
        num_vertices = num_vertices_and_flags & 0x3ff
        flags = num_vertices_and_flags & 0xfc00
        vertex_indices = tuple(parse_vx(s) for v in xrange(num_vertices))
        polygons.append((flags, vertex_indices))
    result['polygons'] = tuple(polygons)
    return result

def parse_ptag(s):
    result = {}
    result['type'] = s.get_bytes(4)
    data = []
    while s:
        data.append((s.unpack('>H')[0], s.unpack('>H')[0]))
    result['data'] = tuple(data)
    return result

def parse_vmad(s):
    result = {}
    result['type'] = s.get_bytes(4)
    dim = s.unpack('>H')[0]
    result['dimension'] = dim
    result['name'] = s.get_string()
    data = []
    while s:
        data.append((parse_vx(s), parse_vx(s), s.unpack('>{0}'.format('f' * dim))))
    result['data'] = tuple(data)
    return result

def parse_surf(s):
    result = {}
    result['name'] = s.get_string()
    result['source'] = s.get_string()
    result['attributes'] = tuple(iter_chunks(s, LWO_SURF_PARSERS, 'H'))
    return result

def parse_colr(s):
    return {'base-color': s.unpack('>fff'), 'envelope': parse_vx(s)}

def parse_base_shading_value(s):
    return {'intensity': s.unpack('>f')[0], 'envelope': parse_vx(s)}

def parse_sman(s):
    return s.unpack('>f')[0]

def parse_side(s):
    return s.unpack('>H')[0]

def parse_blok(s):
    sub_chunks = tuple(iter_chunks(s, LWO_BLOK_PARSERS, 'H'))
    result = {}
    result['header'] = sub_chunks[0]
    result['attributes'] = sub_chunks[1:]
    return result

def block_header_parser(sub_parsers):
    def f(s):
        return {'ordinal': s.get_string(), 'block-attributes': tuple(iter_chunks(s, sub_parsers, 'H'))}
    return f

def parse_block_chan(s):
    return s.get_bytes(4)

def parse_opac(s):
    return {'type': s.unpack('>H')[0], 'opacity': s.unpack('>f')[0], 'envelope': parse_vx(s)}

def parse_u2(s):
    return s.unpack('>H')[0]

def parse_string(s):
    return s.get_string()

def parse_fp4(s):
    return s.unpack('>f')[0]

def parse_fkey(s):
    result = []
    while s:
        result.append((s.unpack('>f')[0], s.unpack('>ffff')))
    return tuple(result)

def parse_ikey(s):
    result = []
    while s:
        result.append(s.unpack('>H')[0])
    return tuple(result)

LWO_SUB_PARSERS = {}
LWO_SUB_PARSERS['FORM'] = form_parser(LWO_SUB_PARSERS)
LWO_SUB_PARSERS['TAGS'] = parse_string_array
LWO_SUB_PARSERS['LAYR'] = parse_layr
LWO_SUB_PARSERS['PNTS'] = parse_pnts
LWO_SUB_PARSERS['BBOX'] = parse_bbox
LWO_SUB_PARSERS['VMAP'] = parse_vmap
LWO_SUB_PARSERS['POLS'] = parse_pols
LWO_SUB_PARSERS['PTAG'] = parse_ptag
LWO_SUB_PARSERS['VMAD'] = parse_vmad
LWO_SUB_PARSERS['SURF'] = parse_surf

LWO_SURF_PARSERS = {}
LWO_SURF_PARSERS['COLR'] = parse_colr
for base_shading_value_type in ('DIFF', 'LUMI', 'SPEC', 'REFL', 'TRAN', 'TRNL'):
    LWO_SURF_PARSERS[base_shading_value_type] = parse_base_shading_value
LWO_SURF_PARSERS['SMAN'] = parse_sman
LWO_SURF_PARSERS['SIDE'] = parse_side
LWO_SURF_PARSERS['BLOK'] = parse_blok

LWO_BLOK_PARSERS = {}
for block_header_type in ('IMAP', 'PROC', 'GRAD', 'SHDR'):
    LWO_BLOK_PARSERS[block_header_type] = block_header_parser(LWO_BLOK_PARSERS)
LWO_BLOK_PARSERS['CHAN'] = parse_block_chan
LWO_BLOK_PARSERS['OPAC'] = parse_opac
LWO_BLOK_PARSERS['ENAB'] = parse_u2
LWO_BLOK_PARSERS['NEGA'] = parse_u2
LWO_BLOK_PARSERS['GRPT'] = parse_u2
LWO_BLOK_PARSERS['PNAM'] = parse_string
LWO_BLOK_PARSERS['INAM'] = parse_string
LWO_BLOK_PARSERS['GRST'] = parse_fp4
LWO_BLOK_PARSERS['GREN'] = parse_fp4
LWO_BLOK_PARSERS['FKEY'] = parse_fkey
LWO_BLOK_PARSERS['IKEY'] = parse_ikey

def parse_lwo(s):
    return tuple(iter_chunks(StringWalker(s), LWO_SUB_PARSERS))

def pretty_print_key_value_pair(key, value, prefix = ''):
    if isinstance(value, dict):
        print '{0}{1}:'.format(prefix, key)
        for sub_key, sub_value in value.items():
            pretty_print_key_value_pair(sub_key, sub_value, prefix + '  ')
    else:
        print '{0}{1}: {2!r}'.format(prefix, key, value)

def pretty_print(parsed, prefix = ''):
    for chunk_id, chunk_data in parsed:
        if chunk_id == 'FORM' and isinstance(chunk_data, tuple):
            print '{0}{1}: {2}'.format(prefix, chunk_id, chunk_data[0])
            pretty_print(chunk_data[1], prefix + '  ')
        else:
            pretty_print_key_value_pair(chunk_id, chunk_data, prefix)

def read_file(filename):
    with open(filename, 'rb') as f:
        return f.read()

if __name__ == '__main__':
    json.dump(parse_lwo(read_file('../models/elfegab.lwo')), sys.stdout)
