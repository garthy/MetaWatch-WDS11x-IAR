from pprint import pprint as pp
import sys

def build(root, string, val):
    node = root 
    for ch in string:
        cur = ch 
        prev_node = node
        node = node.get(cur,None)
        if node is None: 
            prev_node[cur] = {}
            node = prev_node[cur]
    node['value'] = val

def buildtree(charmap):
    root = {}
    for c in charmap:
        build(root, charmap[c], c)
    #pp(root)
    return root
 
def find(string) :
    #print "find:", string
    node = root
    i = 0
    gen = []
    for i in range(len(string)):
        cur = string[i]
        gen.append(cur)
        #print cur
        node = node.get(cur, None)
        #print node
        if node.has_key('value'):
            #print "REST:",string[i+1:]
            #print "STRING:",string[:i+1]
            #print "gen",''.join(gen)
            return {'char' : node['value'],
                    'rest' : string[i+1:]}
    


#class node(object):
#    def __init__(self, r, l):
#        self.r = r
#        self.l = l#
#
#class  leaf(object):
#    def __init__(self, val)
#
#root = None
#def buildtree()
#    for c in char2bit:
#        b = char2bit[c]
#        for bit in b:
#            if root == None:
#                root = node(
    

def textdec(values):
    #print "textdec:"
    bits = []
    for i in values:
        b =  bin(i)[2:]
        b = ((8 - len(b)) * '0') + b
        #print b
            
        bits.append(b)
    bits = ''.join(bits)
    #print bits
    text = []
    while bits:
        ret = find(bits)
        if ret is None:
            break;
        #print ret['char']
        text.append(ret['char'])
        bits = ret['rest']
    return ''.join(text) 
    
    #pp(root)
    

def textenc(charmap, text):
    #print "textenc:"
    bits = []
    for c in text:
        bits.append(charmap[c])
    #print ':'.join(bits)
    bits = ''.join(bits)
    values = []
    #print bits
    while bits:
        s = bits[:8]
        #print "XX:",len(s), s
        s = s + ((8-len(bits[:8])) * '0')
        v = int(s, 2)
        values.append(v)
        bits = bits[8:]

    #print values
    #print len(text)
    #print len(values)
    #print "SAVE:", len(text) - len(values)
    return values

from heapq import heappush, heappop, heapify
from collections import defaultdict
 
def encode(symb2freq):
    """Huffman encode the given dict mapping symbols to weights"""
    heap = [[wt, [sym, ""]] for sym, wt in symb2freq.items()]
    heapify(heap)
    while len(heap) > 1:
        lo = heappop(heap)
        hi = heappop(heap)
        for pair in lo[1:]:
            pair[1] = '0' + pair[1]
        for pair in hi[1:]:
            pair[1] = '1' + pair[1]
        heappush(heap, [lo[0] + hi[0]] + lo[1:] + hi[1:])
    return sorted(heappop(heap)[1:], key=lambda p: (len(p[-1]), p))

#Letter frequencys for english
engfreq = {
' ' :  12.702 * 1.07,
'e' : 12.702,
't' : 9.056,
'a' : 8.167,
'o' : 7.507,
'i' : 6.966,
'n' : 6.749,
's' : 6.327,
'h' : 6.094,
'r' : 5.987,
'd' : 4.253,
'l' : 4.025,
'c' : 2.782,
'u' : 2.758,
'm' : 2.406,
'w' : 2.360,
'f' : 2.228,
'g' : 2.015,
'y' : 1.974,
'p' : 1.929,
'b' : 1.492,
'v' : 0.978,
'k' : 0.772,
'j' : 0.153,
'x' : 0.150,
'q' : 0.095,
'z' : 0.074,
}

def generatetxt():
    s = []
    for i in engfreq:
        val = int(engfreq[i]  * 1000.0)
        #print i,val
        s.append(int(val) * i)
    return ''.join(s)
        
def analyze(txt):
    charmap = {}
    symb2freq = defaultdict(int)
    for ch in txt:
        symb2freq[ch] += 1
        # in Python 3.1+:
        # symb2freq = collections.Counter(txt)
    huff = encode(symb2freq)
    #print "Symbol\tWeight\tHuffman Code"
    for p in huff:
        #print "%s\t%s\t%s" % (p[0], symb2freq[p[0]], p[1])
        charmap[p[0]] = p[1]
    return charmap

if __name__ == '__main__':
    charmap = analyze(generatetxt())
    root = buildtree(charmap)
    v = textenc(charmap, sys.argv[1])
    print v
    #print textdec(v)
