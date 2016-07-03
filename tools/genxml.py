
import os


file = open("test.xml", "w")

file.write("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n")
file.write("<tracks>\n")

for tr in xrange(4):
    file.write("<tr>\n")
    for i in xrange(10000):
        file.write("<m b=\"%s\" L=\"1\">%s</m>\n" % (str(i*20), str(i*20)) )
    
    file.write("</tr>\n")

file.write("</tracks>\n\n")


