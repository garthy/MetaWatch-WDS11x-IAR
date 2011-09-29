import sys
import os.path
from PIL import Image
ICONTYPE = "const unsigned char"

def cleanicondata(icondata):
    newdata = []
    for i in icondata:
        i = i.strip()
        if i:
            newdata.append(int(i, 16))
    return newdata

def c2bmp(cfile):
    dirname = os.path.dirname(cfile)
    dirname = os.path.join(dirname, "bmps")
    try:
        os.mkdir(dirname)
    except WindowsError:
        pass
    data = open(cfile, 'rb').read()
    while 1:
        startpos = data.find(ICONTYPE)
        if startpos == -1:
            break
        data = data[startpos:]

        endoflinepos = data.find('\n')
        icondef = data[:endoflinepos]
        name = icondef.split()[3]
        name, size = name.split('[')
        row, col = size.strip(']').split('*')
        row = int(row)
        col = int(col)
        datastartpos = data.find('{')
        datastoppos = data.find('}')
        icondata = data[datastartpos+1:datastoppos-1]
        icondata = icondata.split(',')
        icondata = cleanicondata(icondata)
        if len(icondata) < (col * row):
            #print name
            #print row,col
            #print len(icondata)
            #print col * row
            #print icondata
            # Add missing data
            print "Missing data",name,  ((col * row) - len(icondata))
            icondata = icondata + [0] * ((col * row) - len(icondata))
            #raise SystemError("ARGH")

        #print icondata
        data = data[datastoppos+1:]
        im = Image.new("1",(col * 8, row),0)
        #print "size = %i;" % ((col) * row)
        #print "ret=  RLE_Compress( %s, out, size );" % (name) 
        #print 'printf("' + name + ' ");'
        #print 'printf("Before:%i After:%i diff:%i\\n", size, ret, size-ret);'
        #print "before += size;"
        #print "after += ret;"
        icondatapos = 0
        for y in range(row):
            for x in range(col):
                d = icondata[x + (col*y)]
                for i in range(8):
                    if d & 1:
                        
                        im.putpixel(((x*8)+i,y),1)
                    d = d >> 1
        im.save(os.path.join(dirname, name + ".bmp"))

if __name__ == '__main__':
    c2bmp(sys.argv[1])
