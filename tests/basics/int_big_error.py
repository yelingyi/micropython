# test errors operating on bignum

i = 1 << 65

try:
    i << -1
except ValueError:
    print("ValueError")

try:
    len(i)
except TypeError:
    print("TypeError")

try:
    1 in i
except TypeError:
    print("TypeError")

# overflow because rhs of >> is being converted to machine int
try:
    1 >> i
except OverflowError:
    print('OverflowError')

# to test conversion of negative mpz to machine int
# (we know << will convert to machine int, even though it fails to do the shift)
try:
    i << (-(i >> 40))
except ValueError:
    print('ValueError')
