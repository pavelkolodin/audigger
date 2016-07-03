
import sys, os
import argparse
import json


encode=False

class PageSplitter:
    def __init__(self, dir_output, size, perpage, name):
        self.dir_output = dir_output
        self.size = size
        self.cnt = 0
        self.page = 0
        self.file = None
        self.perpage = perpage
        self.name = name

    def close(self):
        if self.file:
            self.file.write("""</table></body>""")
            self.file.close()   

    def generateHeader(self):
        ret = """name: <b>%s</b> <a href="/samplebank">root</a>""" % (self.name)
        return ret

    def generatePageLinks(self, page):
        ret = "<p>"
        
        pages = 1 + (self.size-1) / self.perpage
        
        for i in range(1, pages + 1):
            name = "index" if i == 1 else str(i)
            cls = "pgsw" if i != page else "pgswcurrent"
            ret += """<a href={name}.html class="{cls}">{i}</a>""".format(i=i, name=name, cls=cls )
        ret += "</p>"
        return ret

    def nextpage(self):
        self.cnt = 0
        if self.file:
            self.file.write("""</table></body>""")
            self.file.close()

        self.page += 1

        name = "index" if self.page == 1 else str(self.page)
        self.file = open( os.path.join( args.output, name + ".html" ), "wb" )
        self.file.write("""
<!DOCTYPE html>
<html>
<head>
<title>Pashe SampleBank</title>
<meta http-equiv="content-type" content="text/html; charset=utf-8" />
<style>
.maintable td
{
background-color: #eee;
margin-left: 10px;
}
.maintable
{
width: 800px;
}
.pgsw
{
background-color: #ddd;
padding: 5px;
}
.pgswcurrent
{
background-color: #aaf;
padding: 5px;
}
</style>
</head>
<body>
%s
%s
<table cellpadding="1" cellspacing="1" border="0" class="maintable">
<tr>
    <th>play</th>
    <th>ogg</th>
    <th>text</th>
    <th>tags</th>
</tr>
""" % ( self.generateHeader(), self.generatePageLinks(self.page)) )


    def add(self, obj):
        self.cnt += 1
                
        if self.cnt >= self.perpage or None == self.file:
            self.nextpage()

        # <source src="{filename}.ogg" type="audio/ogg">
        element_audio = """<audio controls>
                           <source src="{id}.ogg" type="audio/ogg; codec=vorbis">
                           <embed height="30" width="100" src="{id}.ogg">
                           </audio>
                        """.format(id=obj["id"])

        
        tags = ""
        for word in obj["trackname"].split():
            if '@' == word[0]:
                tags += word[1:]
                tags += ", "
        
            
        s = """<tr><td width="100">%s</td><td><a href="%s.ogg">ogg</a></td><td>%s</td><td>%s</td></tr>""" % (element_audio,
                         obj["id"],
                         obj["text"],
                         tags )
                        
        self.file.write( s.encode('utf-8') )




if '__main__' == __name__:

    parser = argparse.ArgumentParser(description='export')
    parser.add_argument('-i', '--input', dest='input', help='input dir')
    parser.add_argument('-o', '--output', dest='output', help='output dir')
    parser.add_argument('-n', '--name', dest='name', help='name')
    args = parser.parse_args()


    if not os.path.exists( args.output ):
        os.makedirs( args.output )
    
    with open( os.path.join(args.input, "index.json") ) as f:
        index = json.load( f )

    files = list()
    for mark in index['marks']:
        files.append( mark )

        if encode:
            filename_wav = os.path.join(args.input, str(mark["id"]) + ".wav")
            filename_ogg = os.path.join(args.output, str(mark["id"]) + ".ogg")
            cmd = "avconv -y -i %s -acodec libvorbis -qscale:a 7 %s" % (filename_wav, filename_ogg)
        
            #if os.path.exists( filename_ogg ):
            #    continue
        
            print( filename_wav, filename_ogg, cmd )
            os.system( cmd )

    ps = PageSplitter(args.output, len(files), 25, args.name)
    

    for obj in sorted( files, key = lambda x: x["text"] ):
    #for name in sorted( f["text"] for f in files ):
        ps.add( obj )

    ps.close()





