#!/usr/bin/env python3
"""Extract native method declarations from the supplied original APK (DEX)."""
import argparse,struct,zipfile,json,pathlib
p=argparse.ArgumentParser();p.add_argument('apk');p.add_argument('out');a=p.parse_args()
b=zipfile.ZipFile(a.apk).read('classes.dex');u=lambda p:struct.unpack_from('<I',b,p)[0];h=lambda p:struct.unpack_from('<H',b,p)[0]
def leb(p):
 v=0;s=0
 while True:
  c=b[p];p+=1;v|=(c&127)<<s;s+=7
  if c<128:return v,p
ss=[b[leb(u(u(60)+i*4))[1]:].split(b'\0',1)[0].decode('utf8','replace') for i in range(u(56))];ts=[ss[u(u(68)+i*4)] for i in range(u(64))];ms=[(ts[h(p)],ss[u(p+4)],h(p+2)) for p in range(u(92),u(92)+u(88)*8,8)]
rows=[]
for p in range(u(100),u(100)+u(96)*32,32):
 name=ts[u(p)];off=u(p+24)
 if not off or name not in ('Lcom/sixwaves/raftpirates/MyLib;','Lcom/bigbluebubble/hydra/HydraGame;','Lcom/bigbluebubble/hydra/HydraSocial;','Lcom/bigbluebubble/hydrastore/HydraStoreNEW;','Lcom/bigbluebubble/smartfox/ClientServices;'):continue
 counts=[]
 for _ in range(4):v,off=leb(off);counts.append(v)
 for _ in range(counts[0]+counts[1]):v,off=leb(off);v,off=leb(off)
 for count in counts[2:]:
  ix=0
  for _ in range(count):
   v,off=leb(off);ix+=v;flags,off=leb(off);code,off=leb(off)
   if not flags&0x100:continue
   owner,m,pi=ms[ix];pr=u(76)+pi*12;ret=ts[u(pr+4)];po=u(pr+8);args=[ts[h(po+4+2*i)] for i in range(u(po))] if po else []
   rows.append(dict(owner=owner[1:-1],name=m,args=args,returns=ret,static=bool(flags&8),symbol='Java_'+owner[1:-1].replace('_','_1').replace('/','_')+'_'+m.replace('_','_1')))
pathlib.Path(a.out).write_text(json.dumps(rows,indent=2)+'\n');print(len(rows),'native declarations')
