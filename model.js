export const tags=['Reverb','Best Of','Long','Short','Guitar','Vocal','Keys','Drums','Delay','Space','Echo','Chorus','Phase'];
export const decay=[.4,.6,.8,1,1.2,1.5,1.8,2,2.2,2.5,2.8,3,3.3,3.6,4,4.5];
const range=(n,step=1)=>Array.from({length:n},(_,i)=>i*step);
export const modes={
 Reverb:{labels:['Decay · s','LF ×','HF ×','Predelay · ms'],values:[decay,[.5,1,1.5,2],[.25,.33,.5,'max'],[0,20,40,60]],switch:['Front','Rear'],route:'Wet mono → reverb → Front L/R ou Rear L/R; LF a 300 Hz, HF a 6 kHz.'},
 Delay:{labels:['Coarse · ms','Fine · ms','Canal de edição','Predelay · ms'],values:[range(16,20),[0,5,10,15],['I · L','II · L','III · R','IV · R'],[0,20,40,60]],switch:['Stereo','Mono'],route:'Wet mono → predelay comum → delays L/R independentes, sem feedback. Rear soma a saída em mono. Máximo 375 ms/canal.'},
 Phase:{labels:['Phase · ms','Inativo','Inativo','Predelay · ms'],values:[range(16),null,null,[0,20,40,60]],switch:['Front','Rear'],route:'Wet mono → predelay → delay fixo 15 ms + variável 0–15 ms → filtro comb. Sem LFO. Rear: mínimo só R, máximo só L.'},
 Chorus:{labels:['Inativo','Inativo','Variação','Predelay · ms'],values:[null,null,['I','II','III','IV'],[0,20,40,60]],switch:['Stereo','Mono'],route:'Wet mono → predelay → quatro delays modulados aleatoriamente. I: FL/FR; II: RL/RR; III: FL+RL / FR+RR; IV: FL+RL+RR / RL+FR−RR. Rear soma mono.'},
 Echo:{labels:['Coarse · ms','Fine · ms','HF ×','Predelay · ms'],values:[range(16,20),[0,5,10,15],[.25,.33,.5,'max'],[0,20,40,60]],switch:['Input on','Input mute'],route:'Wet mono → mute de entrada → predelay → echo mono com feedback ≈0,9 por volta e damping HF. Predelay fora do feedback; repetições até 315 ms, primeira até 375 ms.'},
 Space:{labels:['Inativo','Inativo','Inativo','Predelay · ms'],values:[null,null,null,[0,20,40,60]],switch:['Front','Rear'],route:'Wet mono → predelay → Space (~10 s, decaimento uniforme por frequência) → Front L/R ou Rear L/R.'}
};
export const defaults={Reverb:[10,1,2,2],Delay:[10,0,0,0],Phase:[7,0,0,0],Chorus:[0,0,0,0],Echo:[12,1,2,0],Space:[0,0,0,0]};
// Curva aproximada; o manual especifica apenas os extremos e o centro = 15%.
export const wetAt=x=>Math.expm1(2*Math.log(17/3)*x)/(Math.pow(17/3,2)-1);
const preset=(name,mode,positions,tagList,mix=.5,rear=false,delay=null)=>({id:name,name,mode,positions,tags:tagList,mix,rear,delay,factory:true});
export const factory=[
 preset('Batman Echo','Echo',[15,3,1,1],['Echo'],1),
 preset('Blessers Barn','Reverb',[6,1,2,1],['Vocal','Reverb'],.62,true),
 preset('Bouncy','Echo',[7,2,2,0],['Echo'],.7),
 preset('Muted Echo','Echo',[10,2,0,0],['Echo'],.75),
 preset('Slapback','Echo',[2,2,0,0],['Echo'],.6),
 preset('Carlos Caverns','Reverb',[14,2,2,2],['Vocal','Reverb'],.65,true),
 ...['I','II','III','IV'].map((v,i)=>preset('Chorus '+v,'Chorus',[0,2,i,0],['Chorus','Guitar','Keys'],1)),
 preset('Clean Space','Reverb',[12,0,1,2],['Vocal','Reverb'],.65),
 preset('Female Vox Plate','Reverb',[8,2,3,0],['Vocal','Reverb'],.6),
 preset('In da Club','Reverb',[10,2,1,3],['Vocal','Reverb'],.65),
 preset('Male Vox Plate','Reverb',[9,3,3,1],['Vocal','Reverb'],.5),
 preset('Default','Reverb',[10,1,2,2],['Reverb'],.5),
 preset('Some Depth','Reverb',[5,1,0,0],['Reverb'],.5),
 preset('Just a Touch','Reverb',[5,2,2,0],['Reverb'],.3,true),
 {...preset('Rear Outputs','Phase',[8,2,2,0],['Phase'],1,true),reference:{source:'screenshot',verified:true,note:'PHAS; LED na marca 160 = 8 ms (escala relativa 0–300 → 0–15 ms); Rear; predelay 0 ms.'}},
 preset('Drum Room','Reverb',[1,2,1,1],['Drums','Reverb'],.65,true),
 preset('Large Room','Reverb',[3,0,1,1],['Drums','Reverb'],.7,true),
 preset('Ocean Way','Reverb',[15,0,0,3],['Long','Reverb','Drums'],.65),
 preset('Small Room','Reverb',[0,2,0,1],['Drums','Reverb'],.65),
 preset('Snare Verb','Reverb',[7,1,3,0],['Drums','Reverb'],.65),
 preset('Soundstage','Reverb',[3,0,1,1],['Drums','Reverb'],.65,true),
 {...preset('Subtle Bloom','Space',[12,0,1,0],['Long'],.4,true),reference:{source:'screenshot',verified:true,note:'SPC/Space; predelay 0 ms; Rear. Faders 1–3 inativos.'}},
 {...preset('Full Bloom','Space',[12,0,1,0],['Long'],1),reference:{source:'screenshot',verified:true,note:'SPC/Space; predelay 0 ms. Faders 1–3 inativos; posições visuais registradas.'}},
 {...preset('Delayed Spacing','Space',[12,0,1,3],['Delay'],.6),reference:{source:'screenshot',verified:true,note:'SPC ativo; tag Delay não determina o modo DSP.'}},
 {...preset('Dual Delay','Delay',[10,0,1,0],['Delay'],.65,false,[200,200]),reference:{source:'screenshot',verified:false,knownChannel:'L',unknown:['delayR'],note:'L=200 ms confirmado; R=200 ms provisório, não visível na foto.'}},
 {...preset('Hands Down Double','Delay',[0,0,1,0],['Delay'],1,false,[0,0]),reference:{source:'screenshot',verified:false,knownChannel:'L',unknown:['delayR'],note:'L=0 ms confirmado; R=0 ms provisório, não visível na foto.'}},
 {...preset('Jimmy Jam Delay','Delay',[3,0,1,0],['Delay'],1,false,[60,60]),reference:{source:'screenshot',verified:false,knownChannel:'L',unknown:['delayR'],note:'L=60 ms confirmado; R=60 ms provisório, não visível na foto.'}}
];
export function snapshot(s){const a=[...s.positions]; modes[s.mode].values.forEach((v,i)=>{if(!v||(s.mode==='Delay'&&i<3))a[i]=null;});return {mode:s.mode,positions:a,delay:s.mode==='Delay'?[...s.delay]:null,rear:s.rear,mix:s.mix};}
export function applyPreset(s,p){s.mode=p.mode; s.positions=[...defaults[p.mode]];p.positions.forEach((v,i)=>{if(v!==null)s.positions[i]=v;});s.delay=p.delay?[...p.delay]:[200,200];s.rear=p.rear;s.mix=p.mix;return s;}
export function delayPositions(s){const channel=s.positions[2]<2?0:1;const t=s.delay[channel];s.positions[0]=Math.floor(t/20);s.positions[1]=(t%20)/5;}
export function setLever(s,i,value){s.positions[i]=value;if(s.mode==='Delay'){if(i===2)delayPositions(s);else if(i<2)s.delay[s.positions[2]<2?0:1]=s.positions[0]*20+s.positions[1]*5;}}

// Associação confirmada pela lista da categoria Long nas fotos.
export const pendingReferences=[];
for(const name of ['Blessers Barn','Carlos Caverns','Clean Space','Default','Delayed Spacing','In da Club','Male Vox Plate']){
 const p=factory.find(p=>p.name===name);
 if(p&&!p.tags.includes('Long'))p.tags.push('Long');
}

// Categoria Guitar confirmada pela lista das referências enviadas.
for(const name of ['Batman Echo','Carlos Caverns','Chorus I','Chorus II','Chorus III','Chorus IV','Slapback']){
 const p=factory.find(p=>p.name===name);
 if(p&&!p.tags.includes('Guitar'))p.tags.push('Guitar');
}

// Best Of: somente os nomes confirmados na lista enviada até agora.
export const bestOfConfirmed=['Blessers Barn','Carlos Caverns','Chorus I','Chorus III','Drum Room','Female Vox Plate','In da Club','Male Vox Plate','Muted Echo','Rear Outputs','Some Depth','Subtle Bloom'];
for(const p of factory){
 p.tags=p.tags.filter(t=>t!=='Best Of');
 if(bestOfConfirmed.includes(p.name))p.tags.push('Best Of');
}

// Tag Space: categoria de presets, independente do modo DSP.
for(const name of ['Clean Space','Delayed Spacing','Full Bloom','Subtle Bloom']){
 const p=factory.find(p=>p.name===name);
 if(p&&!p.tags.includes('Space'))p.tags.push('Space');
}

// Keys: associação confirmada pela lista completa visível nas fotos.
for(const name of ['Chorus I','Chorus II','Chorus III','Chorus IV','Rear Outputs','Some Depth']){
 const p=factory.find(p=>p.name===name);
 if(p&&!p.tags.includes('Keys'))p.tags.push('Keys');
}

// Short: todos os nomes visíveis na lista enviada.
export const shortConfirmed=['Drum Room','Female Vox Plate','Just a Touch','Large Room','Small Room','Snare Verb','Some Depth','Soundstage'];
for(const name of shortConfirmed){
 const p=factory.find(p=>p.name===name);
 if(p&&!p.tags.includes('Short'))p.tags.push('Short');
}

// Primeira remessa da tag Reverb: confirmação por lista, sem limitar continuação.
export const reverbListConfirmed=['Blessers Barn','Carlos Caverns','Clean Space','Drum Room','Female Vox Plate','In da Club','Just a Touch'];
for(const name of reverbListConfirmed){
 const p=factory.find(p=>p.name===name);
 if(p&&!p.tags.includes('Reverb'))p.tags.push('Reverb');
}

export const comparePresets=(a,b)=>a.name.localeCompare(b.name,'en',{numeric:true});
export function mappedPreset(p){
 const mode=modes[p.mode];
 return {
  name:p.name,tags:tags.filter(t=>p.tags.includes(t)),mode:p.mode,
  faders:mode.values.map((values,i)=>({control:i+1,label:mode.labels[i],active:!!values,position:p.positions[i],value:values?values[p.positions[i]]:null})),
  output:mode.switch[Number(p.rear)],predelayMs:mode.values[3][p.positions[3]],
  delayMs:p.mode==='Delay'?{left:p.delay[0],right:p.reference?.unknown?.includes('delayR')?null:p.delay[1]}:null,
  echoMs:p.mode==='Echo'?{repeat:p.positions[0]*20+p.positions[1]*5,first:p.positions[0]*20+p.positions[1]*5+mode.values[3][p.positions[3]]}:null,
  mixPosition:p.mix,mixAccuracy:p.mix===1?'endpoint':'visual-estimate',
  note:p.reference?.note??'',route:mode.route
 };
}
