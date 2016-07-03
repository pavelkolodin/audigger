./samplebank /ar/snd/audigger/gordon-mihalkov /tmp/wav1
./samplebank /ar/snd/audigger/gordon-sobchak-2008-10-03 /tmp/wav2
./samplebank /ar/snd/audigger/yanukovich /tmp/wav3
./samplebank /ar/snd/audigger/zhirinovsky-mironov-piedinok /tmp/wav4
./samplebank /ar/snd/audigger/zhirinovsky-navalny /tmp/wav5
./samplebank /ar/snd/audigger/zhirinovsky-olimpiada-eto-vse-TJMDELSiHOA /tmp/wav6


python2.7 /ar/forge/audigger/samplebank/export.py -i /tmp/wav1 -o /tmp/ogg1 -n "gordon-mihalkov"
python2.7 /ar/forge/audigger/samplebank/export.py -i /tmp/wav2 -o /tmp/ogg2 -n "gordon-sobchak-2008-10-03"
python2.7 /ar/forge/audigger/samplebank/export.py -i /tmp/wav3 -o /tmp/ogg3 -n "yanukovich"
python2.7 /ar/forge/audigger/samplebank/export.py -i /tmp/wav4 -o /tmp/ogg4 -n "zhirinovsky-mironov-piedinok"
python2.7 /ar/forge/audigger/samplebank/export.py -i /tmp/wav5 -o /tmp/ogg5 -n "zhirinovsky-navalny"
python2.7 /ar/forge/audigger/samplebank/export.py -i /tmp/wav6 -o /tmp/ogg6 -n "zhirinovsky-olimpiada-eto-vse-TJMDELSiHOA"
