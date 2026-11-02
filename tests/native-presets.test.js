import {test} from 'node:test';
import assert from 'node:assert/strict';
import {readFileSync} from 'node:fs';
import {factory,tags} from '../model.js';
test('os 30 índices, nomes, tags e parâmetros nativos correspondem ao catálogo',()=>{
 const rows=[...readFileSync(new URL('../native/Presets.h',import.meta.url),'utf8').matchAll(/\{"([^"]+)",(\d+),\{([^}]+)\},([^,]+),(true|false),([^,]+),([^}]+)\}/g)];
 const sorted=[...factory].sort((a,b)=>a.name.localeCompare(b.name,'en'));
 const modes=['Reverb','Delay','Phase','Chorus','Echo','Space'];
 assert.equal(rows.length,30);assert.equal(sorted.length,30);
 rows.forEach((r,i)=>{const p=sorted[i];assert.equal(r[1],`${p.name} [${tags.filter(t=>p.tags.includes(t)).join(', ')}]`);assert.equal(+r[2],modes.indexOf(p.mode));assert.deepEqual(r[3].split(',').map(Number),p.positions,p.name);assert.equal(+r[4],p.mix,p.name);assert.equal(r[5]==='true',p.rear,p.name);assert.equal(+r[6],p.delay?.[0]??200,p.name);assert.equal(+r[7],p.delay?.[1]??200,p.name);});
});
