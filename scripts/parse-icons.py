import sys
import os
import os.path
from PIL import Image
ICONTYPE = "const unsigned char"

header_top = """
//==============================================================================
//  Copyright 2011 Meta Watch Ltd. - http://www.MetaWatch.org/
// 
//  Licensed under the Meta Watch License, Version 1.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  
//      http://www.MetaWatch.org/licenses/license-1.0.html
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//==============================================================================

/******************************************************************************/
/*! \file Icons.h
 *
 * Icons are small bitmaps that are stored in flash.  This is for the LCD only.
 */
/******************************************************************************/

#ifndef ICONS_H
#define ICONS_H

#define LEFT_BUTTON_COLUMN  ( 0 )
#define RIGHT_BUTTON_COLUMN ( 6 )
#define BUTTON_ICON_SIZE_IN_COLUMNS ( 6 )
#define BUTTON_ICON_SIZE_IN_ROWS    ( 32 )

#define BUTTON_ICON_A_F_ROW ( 0 )
#define BUTTON_ICON_B_E_ROW ( 32 )
#define BUTTON_ICON_C_D_ROW ( 64 )

#define IDLE_PAGE_ICON2_STARTING_ROW ( 0 )
#define IDLE_PAGE_ICON2_STARTING_COL ( 10 )
#define IDLE_PAGE_ICON2_SIZE_IN_ROWS ( 30 )
#define IDLE_PAGE_ICON2_SIZE_IN_COLS ( 2 )

#define IDLE_PAGE_ICON_STARTING_ROW ( 10 )
#define IDLE_PAGE_ICON_STARTING_COL ( 8 )
#define IDLE_PAGE_ICON_SIZE_IN_ROWS ( 20 )
#define IDLE_PAGE_ICON_SIZE_IN_COLS ( 2 )

#define LEFT_STATUS_ICON_COLUMN   ( 0 )
#define CENTER_STATUS_ICON_COLUMN ( 4 ) 
#define RIGHT_STATUS_ICON_COLUMN  ( 8 )

#define STATUS_ICON_SIZE_IN_COLUMNS ( 4 )
#define STATUS_ICON_SIZE_IN_ROWS    ( 36 )

#define NUMBER_OF_ROWS_IN_WAVY_LINE ( 5 )
"""

header_bottom = """
#endif /*ICONS_H*/
"""


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
            #print "Missing data",name,  ((col * row) - len(icondata))
            icondata = icondata + [0] * ((col * row) - len(icondata))

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


def bmp2cfile(imagefilename):
    cfile = []
    hfile = []
    name = os.path.basename(imagefilename)
    print name

    im = Image.open(imagefilename)
    width, height = im.size
    if width % 8 != 0 :
        raise SystemError("width must be multiple of 8")
    if height % 8 != 0 :
        raise SystemError("%s: height must be multiple of 8 %i " % (imagefilename, height))
    data = im.getdata()

    array = []

    for y in range(height):
        row = []
        values = []
        for x in range(width):
            pix = data[(y*width) + x]
            if pix:
                row.append("1")
            else:
                row.append("0")
            if len(row) == 8:
                row.reverse()
                values.append(hex(int(''.join(row),2)))
                row = []
        array.append(values)
    hfile.append("extern const unsigned char %s[%i*%i];")
    cfile.append("const unsigned char %s[%i*%i] = {" % (name, height, width/8))
    for row in array:
        row = []
        for item in row:
            row.append(item)
        cfile.append(','.join(row))
    cfile.append("};\n")
    return (hfile,cfile)

def bmp2c(bmpdir):
    header = []
    cfile = []
    print "ARGH",bmpdir
    for bmp in os.listdir(bmpdir):
        print "X",bmp
        (hfile,cfile) = bmp2cfile(os.path.join(bmpdir, bmp))
        header += hfile
        cfile += cfile

    with open('Icon-gen.c','wb') as f:
        f.write('\n'.join(cfile))

    with open('Icon-gen.h','wb') as f:
        f.write(header_top)
        f.write('\n'.join(header))
        f.write(header_bottom)

    

if __name__ == '__main__':
    op = sys.argv[1]
    if op == 'c2bmp':
        c2bmp(sys.argv[2])
    elif op == 'bmp2c':
        bmp2c(sys.argv[2])
    else:
        print "parse-icons.py [c2bmp|bmp2c] "
