#!/usr/bin/env python3

import sys

import random
import string
import struct
import unittest

def encodeInt64(i):
    """ Big-endian encoding, with positive numbers sorting before
        negative numbers.
    """
    mask = 0x8000000000000000
    if (i < 0): i = i & 0xFFFFFFFFFFFFFFFF
    return struct.pack('>Q', i ^ mask)

def encodeBool(b):
    return struct.pack('b', b)

def encodeDouble(d):
    """ https://hbase.apache.org/apidocs/org/apache/hadoop/hbase/util/OrderedBytes.html
        Explains why the algorithm below works. Look for the explanation under
        fixed-length 64-bit float.
    """
    mask = 0xFFFFFFFFFFFFFFFF
    if (d >= 0): mask = 0x8000000000000000
    i = struct.unpack('Q', struct.pack('d', d))[0]
    return struct.pack('>Q', i ^ mask)

def encodeString(s):
    s = bytes(s, 'utf-8')
    return struct.pack('%ds' % len(s), s) + struct.pack('c', bytes(1))

generators = [ lambda: (yield random.randint(-sys.maxsize, sys.maxsize)),
               lambda: (yield random.uniform(sys.float_info.min, sys.float_info.max) * random.choice([1, -1])),
               lambda: (yield random.choice([0, 1])),
               lambda: (yield ''.join(random.choice(string.ascii_lowercase) for i in range(random.randint(1, 100)))) ]

class SerializeTests(unittest.TestCase):

    def test_ints(self):
        for i in range(1000):
            i1 = list(generators[0]())[0]
            i2 = list(generators[0]())[0]
            self.assertEqual(i1 < i2, encodeInt64(i1) < encodeInt64(i2))


    def test_doubles(self):
        for i in range(1000):
            i1 = list(generators[1]())[0]
            i2 = list(generators[1]())[0]
            self.assertEqual(i1 < i2, encodeDouble(i1) < encodeDouble(i2))

    def test_strings(self):
        for i in range(1000):
            i1 = list(generators[3]())[0]
            i2 = list(generators[3]())[0]
            self.assertEqual(i1 < i2, encodeString(i1) < encodeString(i2))

def visualize():
    print(encodeInt64(4))
    print(encodeInt64(-4))
    print(encodeBool(True))
    print(encodeBool(False))
    print(encodeDouble(4.01))
    print(encodeDouble(4.1))
    print(encodeDouble(-4.01))
    print(encodeDouble(-4.1))
    print(encodeString("foo"))

if __name__ == '__main__':
    #visualize()
    unittest.main()
