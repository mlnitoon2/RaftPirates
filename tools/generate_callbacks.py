#!/usr/bin/env python3
import json,re,pathlib
root=pathlib.Path(__file__).resolve().parents[1];rows=json.loads((root/'evidence/jni_inventory.json').read_text());source=(root/'android/OriginalJni.cpp').read_text();prefixes={'com/sixwaves/raftpirates/MyLib':'RP','com/bigbluebubble/hydra/HydraGame':'HY','com/bigbluebubble/smartfox/ClientServices':'SFS'}
types={'V':'void','Z':'jboolean','I':'jint','F':'jfloat','J':'jlong','D':'jdouble','B':'jbyte','S':'jshort','C':'jchar','[F':'jfloatArray','[I':'jintArray','[B':'jbyteArray','Ljava/lang/String;':'jstring'}
def typ(t):return types.get(t,'jobject')
lines=['// Generated from evidence/jni_inventory.json. Unsupported does not mean implemented.'];counts={}
for row in rows:
 prefix=prefixes.get(row['owner']);name=row['name'];impl=bool(prefix and re.search(r'\b'+prefix+r'\('+re.escape(name)+r'\)\s*\(',source));row['status']='implemented_or_partial' if impl else 'unsupported_stub';counts[row['status']]=counts.get(row['status'],0)+1
 if impl:continue
 args=', '.join(['JNIEnv*','jclass' if row['static'] else 'jobject']+[typ(t) for t in row['args']]);r=typ(row['returns']);ret='' if r=='void' else ' return JNI_FALSE;' if r=='jboolean' else ' return nullptr;' if r in ('jobject','jstring') else ' return 0;';lines.append(f'extern "C" JNIEXPORT {r} JNICALL {row["symbol"]}({args}) {{ unsupported("{row["owner"]}.{name}");{ret} }}')
(root/'android/UnsupportedCallbacks.inc').write_text('\n'.join(lines)+'\n');(root/'evidence/jni_coverage.json').write_text(json.dumps(rows,indent=2)+'\n');print(counts)
