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
 'com.bigbluebubble.hydra.HydraGame':'''public static String cache, title, music="", effect="", popup="", webPage=""; public void launchWebPage(String url){webPage=url;} public static int pauses,resumes; public void playMusic(String p){music=p;} public void stopMusic(){music="";} public void pauseMusic(){pauses++;} public void resumeMusic(){resumes++;} public void playSound(String p,float v){effect=p;} public void showPopup(String message,boolean confirm){popup=message;} public String GetCachePath(){return cache;} public String GetFilePath(){return cache;} public void showKeyboardEntry(String t,String v,boolean a,int lo,int hi,boolean l){title=t;} public void showPasswordEntry(String t,String v,String invalid,boolean a,int lo,int hi,boolean l){title=t;}''',
 'com.bigbluebubble.smartfox.ClientServices':'''static final ClientServices INSTANCE=new ClientServices(); public String host,user,token,zone,command;public int port,loginCount;public byte[] bytes;private final sfs2x.client.SmartFox sfs=new sfs2x.client.SmartFox();public void setLoggedIn(boolean value){sfs.loggedIn=value;} public static ClientServices getInstance(){return INSTANCE;} private void init(){} public void Connect(String h,int p){host=h;port=p;} public void Login(String u,String t,String z,com.smartfoxserver.v2.entities.data.SFSObject obj){loginCount++;user=u;token=t;zone=z;bytes=obj.toBinary();} public void SendRequest(String c,com.smartfoxserver.v2.entities.data.SFSObject obj){command=c;bytes=obj.toBinary();} public void Disconnect(){sfs.loggedIn=false;} public void KeepAlive(){}'''}
 for owner in sorted({r['owner'] for r in rows}):
  name=owner.replace('/','.');pkg,cl=name.rsplit('.',1);inherit=' extends com.bigbluebubble.hydra.HydraGame' if cl=='MyLib' else '';methods=[]
  for r in rows:
   if r['owner']==owner:methods.append('public '+('static ' if r['static'] else '')+'native '+typ(r['returns'])+' '+r['name']+'('+','.join(typ(x)+' a'+str(i) for i,x in enumerate(r['args']))+');')
  java(name,'package '+pkg+'; public class '+cl+inherit+' {'+helpers.get(name,'')+'\n'+'\n'.join(methods)+'\n}')
 java('sfs2x.client.entities.User','package sfs2x.client.entities; public class User {}')
 java('sfs2x.client.SmartFox','package sfs2x.client; public class SmartFox {public boolean loggedIn; public boolean isConnected(){return loggedIn;} public sfs2x.client.entities.User getMySelf(){return loggedIn?new sfs2x.client.entities.User():null;}}')
 java('sfs2x.client.core.BaseEvent','''package sfs2x.client.core; import java.util.*; public class BaseEvent {private final Map<String,Object> args;public BaseEvent(Map<String,Object> a){args=a;} public Map getArguments(){return args;}}''')
 java('com.smartfoxserver.v2.entities.data.SFSObject','''package com.smartfoxserver.v2.entities.data; public class SFSObject {private final byte[] data; private SFSObject(byte[] b){data=b;} public static SFSObject newFromBinaryData(byte[] b){return new SFSObject(b);} public byte[] toBinary(){return data;}}''')
 java('org.json.JSONObject','''package org.json; public class JSONObject { private final String text; public JSONObject(String s){text=s;} public boolean optBoolean(String k,boolean d){return k.equals("ok")?text.contains("true"):d;} public String optString(String k,String d){if(k.equals("bbbId"))return "10";if(k.equals("sessId"))return "test-only-session";if(k.equals("serverIp"))return "127.0.0.1";return d;}}''')
 # independent packet writer, enough to form a real typed SFS fixture
 def txt(s):b=s.encode();return struct.pack('>H',len(b))+b
 def val(v):
  if isinstance(v,bool):return b'\1'+bytes([v])
  if isinstance(v,int):return b'\4'+struct.pack('>i',v)
  if isinstance(v,str):return b'\10'+txt(v)
  if isinstance(v,list):return b'\21'+struct.pack('>H',len(v))+b''.join(map(val,v))
  if isinstance(v,dict):return b'\22'+struct.pack('>H',len(v))+b''.join(txt(k)+val(x) for k,x in v.items())
  raise TypeError(v)
 data={'zone_id':1,'map_name':'xml_resources/maps/map_sailers_sea.tmx','map_x':0,'map_y':0,
       'safe':1,'daring_level':0,'daring_timeremaining':0,'daring_paused':0,'daring_totaltime':0,
       'build_limit_left':100,'player_object':{'user_id':1,'display_name':'Player','safe':0,
       'rafts':[{'raft':1,'user_raft_id':1,'pos_x':0,'pos_y':0,'current_bp':1000,'state':0,
        'components':[{'user_raft_component_id':1,'component':2,'current_bp':1000,'flipped':0,
                       'time_remaining_secs':0,'state':0,'pos_x':1,'pos_y':1},
                      {'user_raft_component_id':2,'component':3,'current_bp':1000,'flipped':0,
                       'time_remaining_secs':0,'state':0,'pos_x':0,'pos_y':0}]}],
       'materials':[{'material':m,'amount':99999} for m in (2,3,4,11)],
       'blueprints':[{'blueprint':1}],'gridObjects':[]}}
 (d/'ocean.bin').write_bytes(val({'hidemap':False,'safe_water':False,'eventType':'none',
                                 'discovery_id':0,'ocean_state':{'debris':[]}}))
 (d/'player.bin').write_bytes(val(data))
 (d/'grid.bin').write_bytes(val({'gridobject_data':[{'object_id':1,'name':'CoreRaft','data_string':'0;gfx/rafts/raft_core.png -192 0 0,0;','battle_points':1000},{'object_id':3,'name':'DivingUnit','data_string':'1;xml_bin/anim_core_dive_unit.bin 0 0 0,0;','battle_points':1000}]}))
 java('Smoke','''import java.net.*; import java.io.*; import com.sixwaves.raftpirates.MyLib; import com.bigbluebubble.hydra.HydraGame; import com.bigbluebubble.smartfox.ClientServices; import com.smartfoxserver.v2.entities.data.SFSObject; import sfs2x.client.core.BaseEvent; import java.nio.file.*; import java.util.*;
public class Smoke {
static volatile URL authURL;
static void mockURL() {
 URL.setURLStreamHandlerFactory(protocol -> (protocol.equals("https") || protocol.equals("http")) ? new URLStreamHandler(){
  protected URLConnection openConnection(URL url) {authURL=url; return new HttpURLConnection(url){
   public void connect(){} public void disconnect(){} public boolean usingProxy(){return false;}
   public int getResponseCode(){return 200;} public InputStream getInputStream(){return new ByteArrayInputStream("{true}".getBytes());}
  };}
 } : null);
}
static void check(boolean x,String msg){if(!x)throw new AssertionError(msg);} static BaseEvent ext(String command,Path p)throws Exception{return new BaseEvent(Map.of("cmd",command,"params",SFSObject.newFromBinaryData(Files.readAllBytes(p))));}
public static void main(String[] a)throws Exception{mockURL();System.load(a[0]);HydraGame.cache=a[2];MyLib m=new MyLib();m.initGL();m.init(a[1],640,400,1,1);check(m.isJniInititalized(),"APK init");check(m.getAuthURL().equals("https://raftpirates.6waves.com/"),"original website URL");ClientServices early=ClientServices.getInstance();early.OnLogin(new BaseEvent(Map.of()));early.OnExtensionResponse(ext("gs_player",Path.of(a[2],"player.bin")));early.OnLoginError(new BaseEvent(Map.of()));check(early.host==null&&m.getUserID().isEmpty(),"cold startup ignores stale callbacks");m.tick();m.touchUp(320,200);m.tick();check(HydraGame.music.equals("audio/music/theme.mp3"),"title music");m.init(a[1],640,400,1,1);m.tick();check(HydraGame.music.equals("audio/music/theme.mp3")&&early.host==null,"title survives surface reinit");m.touchDown(320,365);m.touchUp(320,365);m.tick();for(int i=0;i<60;i++){Thread.sleep(30);m.tick();}m.touchDown(320,246);m.touchUp(320,246);m.touchDown(320,194);m.touchUp(320,194);check(HydraGame.title.equals("Email"),"email prompt");m.sendMsgKeyboardEntryResult("test@local.invalid",false);m.touchDown(320,229);m.touchUp(320,229);check(HydraGame.title.equals("Password"),"password prompt");m.sendMsgKeyboardEntryResult("test-only&+",false);m.touchDown(320,266);m.touchUp(320,266);for(int i=0;i<100&&ClientServices.getInstance().host==null;i++){Thread.sleep(30);m.tick();}ClientServices c=ClientServices.getInstance();check("127.0.0.1".equals(c.host)&&c.port==9933,"connect args");check(authURL!=null&&authURL.toString().startsWith(a[3]+"?")&&authURL.getQuery().contains("p=test-only%26%2B"),"properties endpoint and URL encoding");c.OnConnection(new BaseEvent(Map.of("success",true)));check(c.user.equals("10")&&c.token.equals("test-only-session")&&c.zone.equals("RaftPirates"),"login args order");c.OnConnection(new BaseEvent(Map.of("success",true)));check(c.loginCount==1,"duplicate connection ignored");c.OnExtensionResponse(ext("lib_go_data",Path.of(a[2],"grid.bin")));c.OnExtensionResponse(ext("gs_player",Path.of(a[2],"player.bin")));c.OnExtensionResponse(ext("gs_ocean",Path.of(a[2],"ocean.bin")));m.tick();check(c.command==null,"no gameplay request before login event");c.setLoggedIn(true);for(int i=0;i<100;i++){Thread.sleep(30);m.tick();}c.OnAdminMessage(new BaseEvent(Map.of("message","Maintenance soon")));m.tick();check(HydraGame.popup.equals("Maintenance soon"),"admin popup");m.touchUp(30,25);check(c.command.equals("gs_quest"),"HUD quest request");m.touchUp(30,310);m.touchUp(250,372);check(c.command.equals("dive_start"),"game sends diving request");check(c.bytes.length>3 && c.bytes[0]==18,"outgoing SFS object");m.touchState(1,new float[]{2},new float[]{4});m.touchMove(4,5);m.sendMsgMusicFinished();m.sendMsgSoundFinished(2);m.pause(true);m.tick();m.pause(false);check(HydraGame.pauses==1&&HydraGame.resumes==1,"audio pause/resume");c.OnConnectionLost(new BaseEvent(Map.of()));m.tick();check(m.getUserID().isEmpty(),"disconnected user cleared");m.kill();check(HydraGame.music.isEmpty(),"audio stop");check(!m.isJniInititalized(),"shutdown");check(Files.exists(Path.of(a[2],"raftpirates-session.dat")),"session saved");
c.host=null;c.loginCount=0;authURL=null;m.init(a[1],640,400,1,1);m.touchUp(320,200);m.tick();check(c.host==null,"resume presents title first");for(int i=0;i<45;i++){Thread.sleep(30);m.tick();}check(c.host==null,"title remains until Set Sail");m.touchDown(320,365);m.touchUp(320,365);m.tick();check(c.host!=null&&authURL==null,"saved session resumes on Set Sail without HTTP");c.OnConnection(new BaseEvent(Map.of("success",true)));check(c.loginCount==1,"saved session login sent");c.OnLoginError(new BaseEvent(Map.of()));check(!Files.exists(Path.of(a[2],"raftpirates-session.dat")),"rejected saved login removed");m.kill();System.out.println("Original JNI mock lifecycle, login, SFS response and request passed");}}
''')
 subprocess.run(['javac','-h',str(headers),'-d',str(classes),*map(str,src.rglob('*.java'))],check=True)
 # Header generation from the exact DEX descriptors and export check.
 symbols={x.split()[-1] for x in subprocess.check_output(['nm','-D','--defined-only',lib],text=True).splitlines()}
 expected={r['symbol'] for r in rows};assert expected<=symbols,expected-symbols
 # Read the configured authentication endpoint from the original APK. No listener is opened.
 import xml.etree.ElementTree as ET
 with zipfile.ZipFile(apk) as original:
  properties=ET.fromstring(original.read('assets/xml_resources/properties.xml'))
 endpoint=next(x.attrib['value'] for x in properties if x.attrib.get('key')=='SERVER_IP')
 if '://' not in endpoint: endpoint='https://'+endpoint
 if not endpoint.endswith('/auth.php'): endpoint=endpoint.rstrip('/')+'/auth.php'
 subprocess.run(['java','-Xcheck:jni','-cp',str(classes),'Smoke',lib,apk,str(d),endpoint],check=True)
 print('In-process URL fixture: original properties endpoint, URL encoding, saved login and BBB handoff passed')
 print(len(expected),'JNI declarations matched exported names')
