import math

def dot_product(a, b):
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]

def norm_sq(a):
    return dot_product(a, a)

def norm(a):
    return math.sqrt(norm_sq(a))

def normalize(a):
    return scalar_vector_multiply(1.0/norm(a), a)

def cross_product(a, b):
    return (a[1] * b[2] - a[2] * b[1],
            a[2] * b[0] - a[0] * b[2],
            a[0] * b[1] - a[1] * b[0])

def vector_subtract(a, b):
    return (a[0] - b[0], a[1] - b[1], a[2] - b[2])

def scalar_vector_multiply(x, a):
    return (x*a[0], x*a[1], x*a[2])

def remove_polygon_degeneracy(vertices, polygon_indices):
    n = len(polygon_indices)
    wrapped_polygon_indices = polygon_indices * 2
    edge_lengths = [
        norm(vector_subtract(
                vertices[wrapped_polygon_indices[i+1]],
                vertices[wrapped_polygon_indices[i]]))
        for i in xrange(n)]
    polygon_size = sum(edge_lengths)

    result = []
    for i in xrange(n):
        ratio = edge_lengths[i] / polygon_size
        if ratio < 0.000001:
            continue
        result.append(polygon_indices[i])
    return tuple(result)

def check_polygon(vertices, polygon_indices):
    polygon_indices = remove_polygon_degeneracy(vertices, polygon_indices)
    n = len(polygon_indices)
    if n == 2:
        return # We allow length 2 polygons since they represent lines.
    if n < 2:
        raise Exception("Polygon is a point")
    polygon_indices = tuple(polygon_indices)
    polygon_vertices = tuple(vertices[i] for i in polygon_indices)
    wrapped_polygon_vertices = polygon_vertices * 2

    # First check that the polygon is flat and convex.
    normals = []
    for i in xrange(n):
        a = vector_subtract(
            wrapped_polygon_vertices[i+1], wrapped_polygon_vertices[i])
        b = vector_subtract(
            wrapped_polygon_vertices[i+2], wrapped_polygon_vertices[i+1])
        normals.append(cross_product(a, b))
    largest_normal = normals[
        max((norm_sq(normals[i]), i) for i in xrange(n))[1]]
    largest_normal_len = norm(largest_normal)
    if largest_normal_len == 0:
        raise Exception("Polygon is collinear: {0}".format(polygon_vertices))
    unit_normal = normalize(largest_normal)
    for normal in normals:
        if dot_product(normal, unit_normal) <= 0 and n > 4:
            raise Exception(
                ">4 sided polygon not convex: {0}".format(polygon_vertices))
        deviation = norm(cross_product(normal, unit_normal)) / \
            largest_normal_len
        if n > 4 and deviation > 1e-4:
            raise Exception(
                ">4 sided polygon not flat: {0}".format(polygon_vertices))

def polygon_to_triangles(vertices, polygon_indices):
    polygon_indices = remove_polygon_degeneracy(vertices, polygon_indices)
    n = len(polygon_indices)
    if n == 2:
        # Special case: line
        yield (polygon_indices[0], polygon_indices[1], polygon_indices[0])
        return
    if n == 4:
        # Some 4-sided polygons are not flat (or even not convex).
        # Deal with this by ensuring that we fold along the shortest
        # diagonal.
        diagonal_02_len = norm(
            vector_subtract(
                vertices[polygon_indices[0]], vertices[polygon_indices[2]]))
        diagonal_13_len = norm(
            vector_subtract(
                vertices[polygon_indices[1]], vertices[polygon_indices[3]]))
        if diagonal_13_len < diagonal_02_len:
            polygon_indices = polygon_indices[1:] + (polygon_indices[0],)
    # Now we can do a naive conversion where we make a sequence of
    # triangles sharing the 0th vertex.
    for i in xrange(1, n-1):
        yield (polygon_indices[0], polygon_indices[i], polygon_indices[i+1])

def polygons_to_triangles(vertices, polygons):
    for polygon_indices in polygons:
        for triangle in polygon_to_triangles(vertices, polygon_indices):
            yield triangle
