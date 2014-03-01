def _dict_substructure(A, B):
    for key, val in A.iteritems():
        if not key in B:
            return False
        if not (substructure(val, B[key])):
            return False
    return True 

def _list_substructure(A, B):
    if len(B) < len(A):
        return False
    for a, b in zip(A, B):
        if not substructure(a, b):
            return False
    return True

def substructure(A, B):
    if isinstance(A, set) and isinstance(B, set):
        return A.issubset(B)
    if isinstance(A, set) and isinstance(B, dict):
        return A.issubset(set(B.keys()))
    if isinstance(A, set) and isinstance(B, list):
        return A.issubset(set(B))
    if isinstance(A, dict) and isinstance(B, dict):
        return _dict_substructure(A, B)
    if isinstance(A, list) and isinstance(B, list):
        return _list_substructure(A, B)
    return (A is None) or (A == B)
