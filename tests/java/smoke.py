#!/usr/bin/env python3
"""Desktop JNI integration test. Test doubles match descriptors, not the real Android runtime."""
import pathlib, subprocess, tempfile, json, struct, sys, zipfile
root=pathlib.Path(__file__).resolve().parents[2]
lib,apk=map(lambda x:str(pathlib.Path(x).resolve()),sys.argv[1:3]); rows=json.loads((root/'evidence/jni_inventory.json').read_text())
T={'V':'void','Z':'boolean','I':'int','F':'float','J':'long','D':'double','B':'byte','S':'short','C':'char','[F':'float[]','[B':'byte[]','[I':'int[]'}
def typ(t):return T.get(t,t[1:-1].replace('/','.'))
with tempfile.TemporaryDirectory(prefix='hydra-jni-') as d:
 d=pathlib.Path(d);src=d/'src';classes=d/'classes';classes.mkdir();headers=d/'headers';headers.mkdir()
 def java(name,body):
  p=src/(name.replace('.','/')+'.java');p.parent.mkdir(parents=True,exist_ok=True);p.write_text(body)
 helpers={
 'com.bigbluebubble.hydra.HydraGame':'''public static String cache, title; public String GetCachePath(){return cache;} public void showKeyboardEntry(String t,String v,boolean a,int lo,int hi,boolean l){title=t;} public void showPasswordEntry(String t,String v,String invalid,boolean a,int lo,int hi,boolean l){title=t;}''',
 'com.bigbluebubble.smartfox.ClientServices':'''static final ClientServices INSTANCE=new ClientServices(); public String host,user,token,zone,command;public int port;public byte[] bytes; public static ClientServices getInstance(){return INSTANCE;} private void init(){} public void Connect(String h,int p){host=h;port=p;} public void Login(String u,String t,String z,com.smartfoxserver.v2.entities.data.SFSObject obj){user=u;token=t;zone=z;bytes=obj.toBinary();} public void SendRequest(String c,com.smartfoxserver.v2.entities.data.SFSObject obj){command=c;bytes=obj.toBinary();} public void Disconnect(){}'''}
 for owner in sorted({r['owner'] for r in rows}):
  name=owner.replace('/','.');pkg,cl=name.rsplit('.',1);inherit=' extends com.bigbluebubble.hydra.HydraGame' if cl=='MyLib' else '';methods=[]
  for r in rows:
   if r['owner']==owner:methods.append('public '+('static ' if r['static'] else '')+'native '+typ(r['returns'])+' '+r['name']+'('+','.join(typ(x)+' a'+str(i) for i,x in enumerate(r['args']))+');')
  java(name,'package '+pkg+'; public class '+cl+inherit+' {'+helpers.get(name,'')+'\n'+'\n'.join(methods)+'\n}')
 java('sfs2x.client.core.BaseEvent','''package sfs2x.client.core; import java.util.*; public class BaseEvent {private final Map<String,Object> args;public BaseEvent(Map<String,Object> a){args=a;} public Map getArguments(){return args;}}''')
 java('com.smartfoxserver.v2.entities.data.SFSObject','''package com.smartfoxserver.v2.entities.data; public class SFSObject {private final byte[] data; private SFSObject(byte[] b){data=b;} public static SFSObject newFromBinaryData(byte[] b){return new SFSObject(b);} public byte[] toBinary(){return data;}}''')
 # independent packet writer, enough to form a real typed SFS fixture
 def txt(s):b=s.encode();return struct.pack('>H',len(b))+b
 def val(v):
  if isinstance(v,bool):return b'\1'+bytes([v])
  if isinstance(v,int):return b'\4'+struct.pack('>i',v)
  if isinstance(v,str):return b'\10'+txt(v)
  if isinstance(v,list):return b'\21'+struct.pack('>H',len(v))+b''.join(map(val,v))
  if isinstance(v,dict):return b'\22'+struct.pack('>H',len(v))+b''.join(txt(k)+val(x) for k,x in v.items())
  raise TypeError(v)
 data={'player_object':{'display_name':'Test','rafts':[{'user_raft_id':1,'raft':1,'pos_x':0,'pos_y':0,'components':[{'user_raft_component_id':2,'component':3,'pos_x':1,'pos_y':1}]}],'materials':[],'blueprints':[]},'zone_id':1,'safe':1,'daring_level':1}
 (d/'player.bin').write_bytes(val(data));(d/'grid.bin').write_bytes(val({'gridobject_data':[{'object_id':1,'name':'CoreRaft','data_string':'0;gfx/rafts/raft_core.png -192 0 0,0;','battle_points':1000},{'object_id':3,'name':'DivingUnit','data_string':'1;xml_bin/anim_core_dive_unit.bin 0 0 0,0;','battle_points':1000}]}))
 java('Smoke','''import com.sixwaves.raftpirates.MyLib; import com.bigbluebubble.hydra.HydraGame; import com.bigbluebubble.smartfox.ClientServices; import com.smartfoxserver.v2.entities.data.SFSObject; import sfs2x.client.core.BaseEvent; import java.nio.file.*; import java.util.*;
public class Smoke {
static void check(boolean x,String msg){if(!x)throw new AssertionError(msg);} static BaseEvent ext(String command,Path p)throws Exception{return new BaseEvent(Map.of("cmd",command,"params",SFSObject.newFromBinaryData(Files.readAllBytes(p))));}
public static void main(String[] a)throws Exception{System.load(a[0]);HydraGame.cache=a[2];MyLib m=new MyLib();m.initGL();m.init(a[1],640,400,1,1);check(m.isJniInititalized(),"APK init");m.tick();m.touchUp(320,200);m.tick();m.touchDown(320,365);m.touchUp(320,365);m.tick();m.touchUp(40,370);check(HydraGame.title.contains("hostname"),"host prompt");m.sendMsgKeyboardEntryResult("127.0.0.1",false);check(HydraGame.title.contains("BBB ID"),"ID prompt");m.sendMsgKeyboardEntryResult("10",false);check(HydraGame.title.contains("session"),"token prompt");m.sendMsgKeyboardEntryResult("test-only",false);ClientServices c=ClientServices.getInstance();check(c.host.equals("127.0.0.1")&&c.port==9933,"connect args");c.OnConnection(new BaseEvent(Map.of("success",true)));check(c.user.equals("10")&&c.token.equals("test-only")&&c.zone.equals("RaftPirates"),"login args order");c.OnLogin(new BaseEvent(Map.of()));c.OnExtensionResponse(ext("lib_go_data",Path.of(a[2],"grid.bin")));c.OnExtensionResponse(ext("gs_player",Path.of(a[2],"player.bin")));m.tick();m.touchUp(30,25);check(c.command.equals("gs_quest"),"HUD quest request");m.touchUp(30,310);m.touchUp(250,372);check(c.command.equals("dive_start"),"game sends diving request");check(c.bytes.length>3 && c.bytes[0]==18,"outgoing SFS object");m.touchState(1,new float[]{2},new float[]{4});m.touchMove(4,5);m.sendMsgMusicFinished();m.sendMsgSoundFinished(2);m.pause(true);m.tick();m.pause(false);m.kill();check(!m.isJniInititalized(),"shutdown");System.out.println("Original JNI mock lifecycle, login, SFS response and request passed");}}
''')
 subprocess.run(['javac','-h',str(headers),'-d',str(classes),*map(str,src.rglob('*.java'))],check=True)
 # Header generation from the exact DEX descriptors and export check.
 symbols={x.split()[-1] for x in subprocess.check_output(['nm','-D','--defined-only',lib],text=True).splitlines()}
 expected={r['symbol'] for r in rows};assert expected<=symbols,expected-symbols
 subprocess.run(['java','-Xcheck:jni','-cp',str(classes),'Smoke',lib,apk,str(d)],check=True)
 print(len(expected),'JNI declarations matched exported names')
