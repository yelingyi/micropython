# test start and end pos specification

try:
    import re
except ImportError:
    print("SKIP")
    raise SystemExit


def print_groups(match):
    print("----")
    try:
        if match is not None:
            i = 0
            while True:
                print(match.group(i))
                i += 1
    except IndexError:
        pass

p = re.compile(r"o")
m = p.match("dog")
print_groups(m)

m = p.match("dog", 1)
print_groups(m)

m = p.match("dog", 2)
print_groups(m)

m = p.match("dog", 3) # Past end of input
print_groups(m)

m = p.match("dog", 0, 1)
print_groups(m)

# Caret only matches the actual beginning
p = re.compile(r"^o")
m = p.match("dog", 1)
print_groups(m)

# End at begging means searching empty string
p = re.compile(r"o")
m = p.match("dog", 1, 1)
print_groups(m)

# Search also works
print('--search')

p = re.compile(r"o")
m = p.search('dog')
print_groups(m)

m = p.search('dog', 1)
print_groups(m)

m = p.search('dog', 2)
print_groups(m)

