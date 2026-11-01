import {writeFileSync} from 'node:fs';
import {factory,tags,comparePresets,mappedPreset} from '../model.js';
const presets=[...factory].sort(comparePresets).map(mappedPreset);
const data={schemaVersion:1,description:'Mapeamento de controles pelas fotos e manual. Sem DSP. Tempos R desconhecidos exportados como null. Wet Solo e Noise globais; ambos iniciam ligados.',tags:Object.fromEntries(tags.map(t=>[t,presets.filter(p=>p.tags.includes(t)).map(p=>p.name)])),presets};
writeFileSync('presets.json',JSON.stringify(data,null,2)+'\n');
const rows=[['Preset','Tags','Modo','Fader 1','Fader 2','Fader 3','Predelay ms','Saída','Dry/Wet curso (aproximado)','Observação'],...presets.map(p=>[p.name,p.tags.join(', '),p.mode,...p.faders.slice(0,3).map(f=>f.active?`${f.label}: ${f.value}`:'Inativo'),p.predelayMs,p.output,p.mixPosition,p.note])];
writeFileSync('MAPEAMENTO.csv','\ufeff'+rows.map(r=>r.map(v=>'"'+String(v).replaceAll('"','""')+'"').join(';')).join('\n')+'\n');
console.log(`${presets.length} presets exportados; ${tags.length} tags.`);
