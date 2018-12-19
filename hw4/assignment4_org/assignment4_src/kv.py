#!/usr/bin/python

# Copyright 2013-present Barefoot Networks, Inc. 
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from scapy.all import sniff, sendp
from scapy.all import Packet
from scapy.all import ShortField, IntField, LongField, BitField, ByteField

import networkx as nx

import sys

class KeyValue(Packet):
    name = "KeyValue"
    fields_desc = [
        LongField("preamble", 0),
        IntField("num_valid", 0),
        ByteField("port", 0),
        ByteField("mtype", 0),
        IntField("key", 0),
        IntField("value", 0),
    ]

def main():

    while(1):
        msg = raw_input("What do you want to send: ")

        # finding the route
        first = None
        inp = msg.split(" ")
        p = None
        type = 0
        if inp[0] == 'put':
            type = 1
            p = KeyValue(preamble = 1, num_valid = 1, port = 127, mtype= type, key=int(inp[1]), value=int(inp[2]))
        else:
            p = KeyValue(preamble = 1, num_valid = 1, port = 127, mtype= type, key=int(inp[1]))
        print p.show()
        sendp(p, iface = "eth0")
        # print msg

if __name__ == '__main__':
    main()