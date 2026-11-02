Download Windows x64 **VST3 + VST2 0.4.0**: [EMT-250-Reverb-0.4.0-Windows-x64-VST3-VST2.zip](releases/EMT-250-Reverb-0.4.0-Windows-x64-VST3-VST2.zip). Instalação e diagnóstico: [INSTALAR-WINDOWS.txt](INSTALAR-WINDOWS.txt). Interface Win32 compartilhada com suporte a DPI e escalas personalizadas, seis modos DSP e 30 presets. Painel redesenhado conforme a última referência, com capas e sanfonas 3D, texturas e suporte a escala sem deformação. Versão gráfica testada via Wine; o usuário confirmou o funcionamento da 0.3.0 no REAPER 7.84 x64.

Captura do painel a 125% em host de teste via Wine: [interface 0.4.0](releases/interface-0.4.0-125.png).

Botão direito no painel: presets, comparação A/B, COPY/PASTE, arquivos `.a250` e INFO. Clique no nome EMT 250 para abrir a biblioteca.

# EMT-250 Reverb — VST3, VST2 e biblioteca de presets

Versão nativa experimental: consulte [NATIVE.md](NATIVE.md). Contém seis modos DSP aproximados próprios e 30 presets mapeados. VST3 e VST2 Windows compartilham o novo painel; Linux conserva o editor genérico. A interface web abaixo é um protótipo separado. Não é uma reprodução exata do algoritmo EMT/UAD.

Tabela consolidada: [MAPEAMENTO.csv](MAPEAMENTO.csv). Dados: [presets.json](presets.json). `npm run export` atualiza ambos; `node scripts/native-presets.js` atualiza os presets nativos.

# EMT 250 — protótipo de controles

Execute `npm start` em `/workspace/EMT-250-reverb-digital-` e abra a porta 8000 no seu ambiente de desenvolvimento. Sem dependências externas. Testes: `npm test`.

Esta interface web implementa controles e presets; o DSP e os plugins nativos estão em `native/`. A interface web **não processa áudio nem carrega VST/AU**. A topologia de áudio é descrita na interface para orientar a implementação posterior. Não reproduz os algoritmos proprietários, conversores ou ruído do hardware.

Clique no nome do preset para abrir a biblioteca. Busca textual, tags com união (OU), favoritos e presets locais são persistidos no localStorage do navegador. SAVE cria um preset; DELETE e RENAME atuam apenas nos presets criados pelo usuário. Tags selecionadas são atribuídas ao salvar. Presets de fábrica são aproximações criativas a partir dos nomes visíveis nas referências, não cópias dos presets UAD. Wet Solo e Noise são globais e iniciam ligados; não são alterados ao carregar presets.

Os quatro faders usam apenas passos permitidos pelo manual. Delay mantém tempos L/R independentes; o terceiro fader seleciona o canal e não é salvo. Parâmetros de modos diferentes são lembrados ao alternar modos. Controles inativos ficam desativados e são omitidos ao salvar. Dry/Wet usa curva aproximada com extremos 0–100% e centro 15%.

## Conexões

Entrada estéreo → ramo dry estéreo; ramo wet soma L+R em mono → Power/Noise e processamento do modo → seleção/soma de saídas → Wet Solo/Dry-Wet → saída estéreo. Power desligado representa bypass. Register é indicador de entrada em +6 dB sobre a referência histórica; não há medição real neste protótipo.

| Modo | Fader 1 | Fader 2 | Fader 3 | Fader 4 | Front/Rear |
|---|---|---|---|---|---|
| Reverb | 0,4–4,5 s, 16 passos | LF ×0,5/1/1,5/2 a 300 Hz | HF ×0,25/0,33/0,5/max a 6 kHz | 0/20/40/60 ms | Par Front ou Rear |
| Delay | 0–300 ms, passos de 20 | 0/5/10/15 ms | I/II = L; III/IV = R | Comum L/R | Stereo / soma mono |
| Phase | 0–15 ms, passos de 1 | Inativo | Inativo | Comum | Par Front ou Rear |
| Chorus | Inativo | Inativo | I/II/III/IV | Comum | Stereo / soma mono |
| Echo | 0–300 ms, passos de 20 | 0/5/10/15 ms | HF ×0,25/0,33/0,5/max | Somente antes do loop | Entrada normal / mute, cauda preservada |
| Space | Inativo | Inativo | Inativo | 0/20/40/60 ms | Par Front ou Rear |

Delay: sem feedback; até 375 ms por canal incluindo predelay. Echo: feedback sempre ativo, ≈10% de atenuação a cada volta; repetições até 315 ms, primeiro eco até 375 ms. Phase: delay fixo de 15 ms combinado com variável, sem LFO; Rear no mínimo só R, no máximo só L. Space: decaimento aproximado de 10 s uniforme por frequência.

Chorus: I = FL/FR; II = RL/RR; III = (FL+RL)/(FR+RR); IV = (FL+RL+RR)/(RL+FR−RR). Quatro delays modulados aleatoriamente. Os controles e rotas são baseados no manual enviado pelo usuário.

Parâmetros para futura automação: decay, LF/HF reverb, delayL/delayR (tempos combinados, não coarse/fine separados), phase, chorusVariation, echoTime (combinado), echoHF, predelay, frontRear, power, noise, dryWet e wetSolo. Seletor de canal Delay é apenas estado de interface. Automação de host ainda não implementada.

### Referências Chorus recebidas

Chorus I/II/III/IV: terceiro fader nas posições I/II/III/IV respectivamente, predelay 0 ms, Front (Stereo), Dry/Wet no máximo. As fotos mostram Wet Solo e Noise ligados; estes controles continuam globais, sem sobrescrever a escolha do usuário ao carregar um preset. Tags: Chorus, Best Of, Guitar, Keys. Faders 1 e 2: posição visual inferior e aproximadamente terceira posição, respectivamente; sem efeito sonoro no modo Chorus. O protótipo mantém esses controles desativados.

### Referências da tag Delay

A tag é uma categoria de biblioteca, independente do modo DSP. Delayed Spacing: SPC/Space ativo na foto, predelay 60 ms; posições dos controles inativos preservadas como referência visual. Dry/Wet aproximado em 60% do curso. Dual Delay: DEL, canal L selecionado, coarse 200 ms, fine 0 ms, predelay 0 ms; mix aproximado em 65% do curso. Hands Down Double: DEL, L 0+0 ms, predelay 0 ms, mix no máximo. Jimmy Jam Delay: DEL, L 60+0 ms, predelay 0 ms, mix no máximo. Front nas quatro referências.

Os tempos R dos três presets DEL não são visíveis: valores provisórios iguais a L, explicitamente identificados na interface e nos dados (`reference.unknown`). Para reproduzir os presets completos, falta uma foto com III-R/IV selecionado em cada um. Long e Space aparecem como filtros relacionados, sem evidência da atribuição individual a cada preset. Large Courtyard aparece na lista, mas ainda não há imagem dos seus controles; não recebeu um mapeamento inventado.

### Referências da tag Vocal

| Preset | Decay | LF | HF | Predelay | Par |
|---|---|---|---|---|---|
| Blessers Barn | 1,8 s | ×1 | ×0,5 | 20 ms | Rear |
| Carlos Caverns | 4,0 s | ×1,5 | ×0,5 | 40 ms | Rear |
| Clean Space | 3,3 s | ×0,5 | ×0,33 | 40 ms | Front |
| Female Vox Plate | 2,2 s | ×1,5 | max | 0 ms | Front |

Todos em REV. Leitura dos passos pelos LEDs; Dry/Wet aproximado visualmente (62%, 65%, 65%, 60% do curso, respectivamente), não valores numéricos oficiais. Wet Solo e Noise ligados nas fotos, mantendo a política global da instância. Os botões Reverb/Best Of/Long/Short/Guitar/Space são filtros relacionados da categoria, sem atribuição individual completa inferível. In da Club e Male Vox Plate foram acrescentados a partir das fotos seguintes. A nova referência Vocal de Clean Space substitui a aproximação anterior em Space.

Complemento Vocal: In da Club = REV, decay 2,8 s, LF ×1,5, HF ×0,33, predelay 60 ms, Front, Dry/Wet aproximadamente 65% do curso. Male Vox Plate = REV, decay 2,5 s, LF ×2, HF max, predelay 20 ms, Front, Dry/Wet aproximadamente 50% do curso (15% wet na curva aproximada). Wet Solo e Noise aparecem ligados nas duas fotos; continuam controles globais da instância. A tag Vocal contém seis presets, na ordem das referências.

### Referências da tag Long

A lista visível confirma Long para Blessers Barn, Carlos Caverns, Clean Space, Default, Delayed Spacing, Full Bloom e In da Club. Tags são adicionadas aos mesmos presets, sem duplicações e preservando Vocal/Delay. As quatro fotos de controles confirmam os valores já registrados para Blessers Barn, Carlos Caverns e Clean Space; Default = REV, 2,8 s, LF ×1, HF ×0,5, predelay 40 ms, Front, mix aproximadamente no centro. Full Bloom foi confirmado pelas fotos seguintes e deixou de ser pendente.

Complemento Long: Full Bloom = SPC/Space (~10 s fixos), predelay 0 ms, Front, Dry/Wet aproximadamente no centro. Delayed Spacing confirmado SPC/Space, predelay 60 ms, Front; posições visuais inativas agora [10,1,2] conforme nova foto. Faders 1–3 não alteram o áudio em Space. In da Club permanece REV 2,8 s / LF ×1,5 / HF ×0,33 / 60 ms. Male Vox Plate recebe também Long e mantém REV 2,5 s / LF ×2 / HF max / 20 ms. Long contém oito presets confirmados, sem duplicação.

Novas referências Long: Ocean Way = REV, decay 4,5 s, LF ×0,5, HF ×0,25, predelay 60 ms, Front, Dry/Wet aproximadamente 65% do curso. Subtle Bloom = SPC/Space (decaimento fixo ~10 s), predelay 0 ms, Rear, Dry/Wet aproximadamente 40% do curso; posições dos faders inativos [15,0,0] registradas apenas visualmente. Long agora reúne dez presets confirmados.

### Referências da tag Echo

| Preset | Coarse | Fine | HF | Predelay | Primeiro eco | Repetições |
|---|---|---|---|---|---|---|
| Batman Echo | 300 ms | 15 ms | ×0,33 | 20 ms | 335 ms | 315 ms |
| Bouncy | 140 ms | 10 ms | ×0,5 | 0 ms | 150 ms | 150 ms |
| Muted Echo | 200 ms | 10 ms | ×0,25 | 0 ms | 210 ms | 210 ms |
| Slapback | 40 ms | 10 ms | ×0,25 | 0 ms | 50 ms | 50 ms |

Os quatro estão no modo ECHO, Front (entrada habilitada). Muted Echo descreve damping escuro, sem inferir mute da entrada pelo nome. Dry/Wet aproximado pelo curso: 100%, 70%, 75%, 60%, respectivamente. Wet Solo e Noise aparecem ligados, mantidos como controles globais. Bouncy substitui a estimativa anterior em Delay e passa a pertencer a Echo. Best Of e Guitar aparecem como filtros relacionados, sem atribuição individual confirmada, e não são acrescentados automaticamente a todos os presets.

### Referências da tag Drums

| Preset | Decay | LF | HF | Predelay | Par |
|---|---|---|---|---|---|
| Drum Room | 0,6 s | ×1,5 | ×0,33 | 20 ms | Rear |
| Large Room | 1,0 s | ×0,5 | ×0,33 | 20 ms | Rear |
| Ocean Way | 4,5 s | ×0,5 | ×0,25 | 60 ms | Front |
| Small Room | 0,4 s | ×1,5 | ×0,25 | 20 ms | Front |

Todos no modo REV. Ocean Way compartilha as tags Long, Reverb e Drums, sem duplicar seu registro. Dry/Wet aproximado visualmente em 65%, 70%, 65%, 65% do curso, respectivamente. Wet Solo e Noise ligados nas fotos, preservados como controles globais. Snare Verb e Soundstage foram confirmados pelas fotos seguintes e removidos das referências pendentes. Best Of/Long/Short são filtros relacionados; não foram atribuídos indiscriminadamente a todos os presets Drums.

Complemento Drums: Snare Verb = REV, decay 2,0 s, LF ×1, HF max, predelay 0 ms, Front. Soundstage = REV, decay 1,0 s, LF ×0,5, HF ×0,33, predelay 20 ms, Rear. Dry/Wet aproximado em 65% do curso para ambos. Wet Solo e Noise ligados nas imagens, preservados como controles globais. A tag Drums agora contém os seis presets enviados, na ordem da lista, sem duplicações.

### Referências da tag Guitar

Lista confirmada: Batman Echo, Carlos Caverns, Chorus I, Chorus II, Chorus III, Chorus IV e Slapback. A categoria é acrescentada aos registros existentes, preservando Echo, Vocal, Long e Chorus. As quatro fotos detalham os quatro primeiros: Batman Echo mantém 300+15 ms, HF ×0,33 e predelay 20 ms; Carlos Caverns mantém REV 4,0 s, LF ×1,5, HF ×0,5, predelay 40 ms, Rear; Chorus I/II mantêm variações I/II e predelay 0 ms. Posição diferente do fader 1 em Chorus não altera parâmetro: é inativo nesse modo. Chorus III/IV e Slapback foram associados pela lista e mantêm as configurações das fotos anteriores.

### Referências da tag Best Of

Confirmados na lista recebida: Blessers Barn, Carlos Caverns, Chorus I, Chorus III, Drum Room e Female Vox Plate. Os registros são compartilhados com suas outras tags, sem duplicações e sem alterar parâmetros já confirmados. Removida a atribuição estimada de Best Of para Chorus II/IV; agora somente evidência da lista define essa categoria. As quatro fotos detalhadas confirmam os controles ativos já registrados dos quatro primeiros. A lista pode continuar abaixo da área visível; seis é o total confirmado até aqui, não uma declaração de categoria completa.

Complemento Best Of: fotos confirmam Drum Room (REV 0,6 s, LF ×1,5, HF ×0,33, predelay 20 ms, Rear), Female Vox Plate (REV 2,2 s, LF ×1,5, HF max, 0 ms, Front) e acrescentam In da Club (REV 2,8 s, LF ×1,5, HF ×0,33, 60 ms, Front). Sete presets confirmados em Best Of até aqui. Configurações anteriores mantidas; somente associação de In da Club foi acrescentada.

Complemento da sequência Best Of: Male Vox Plate e Muted Echo confirmam seus ajustes anteriores e recebem Best Of sem duplicação. Novo Rear Outputs = PHAS/Phase, 8 ms de deslocamento (LED na marca impressa 160, escala relativa 0–300 para 0–15 ms), predelay 0 ms, Rear, Dry/Wet no máximo. Faders 2 e 3 inativos no modo Phase, posições visuais registradas em [2,2] sem efeito de áudio e omitidas na serialização. Filtro Phase acrescentado à biblioteca. A associação Best Of destas três fotos segue a continuidade da sequência enviada pelo usuário; o painel de tags não está visível nestas imagens. Dez presets confirmados na sequência Best Of até aqui.

Continuação Best Of: Some Depth = REV, decay 1,5 s, LF ×1, HF ×0,25, predelay 0 ms, Front, Dry/Wet aproximadamente no centro. Subtle Bloom permanece SPC/Space, predelay 0 ms, Rear, mix aproximadamente 40% do curso; posições visuais inativas atualizadas para [5,1,0] conforme a foto mais recente, sem alteração do comportamento. Subtle Bloom conserva Long e recebe Best Of. Associação Best Of segue a sequência da conversa; painel de tags ausente nestas duas fotos. Doze presets nessa sequência até aqui.

### Referências da tag Space

Quatro presets na lista: Clean Space, Delayed Spacing, Full Bloom e Subtle Bloom. Tags anteriores preservadas, sem registros duplicados. Clean Space continua REV: 3,3 s, LF ×0,5, HF ×0,33, predelay 40 ms, Front. Delayed Spacing continua SPC: predelay 60 ms, Front. Full Bloom continua SPC: predelay 0 ms, Front, Dry/Wet agora no máximo conforme foto mais recente. As posições visuais inativas de Delayed Spacing/Full Bloom são [12,0,1], sem efeito sobre o processamento em Space. Subtle Bloom conserva SPC, predelay 0 ms, Rear conforme referência anterior; nesta remessa aparece somente na lista. Dry/Wet dos demais permanece uma aproximação visual.

Foto final Space/Subtle Bloom: confirma SPC, predelay 0 ms, Rear e Dry/Wet aproximadamente em 35–40% do curso (mantido 40% aproximado). Posições visuais inativas atualizadas para [12,0,1], sem alterar parâmetros sonoros. Categoria Space completa com quatro presets; Subtle Bloom preserva também Long e Best Of.

### Referências da tag Keys

Lista confirmada: Chorus I, Chorus II, Chorus III, Chorus IV, Rear Outputs e Some Depth. Adicionada Keys a Rear Outputs e Some Depth; Chorus I–IV já possuíam a tag. Registros compartilhados preservam demais categorias e parâmetros. Fotos detalhadas confirmam Chorus I/II/III com suas variações correspondentes, predelay 0 ms e Front; faders 1/2 são inativos no modo Chorus, e suas posições visuais diferentes não indicam mudança sonora. Chorus IV, Rear Outputs e Some Depth aparecem na lista e mantêm os mapeamentos confirmados nas referências anteriores.

### Referências da tag Short

Lista confirmada: Drum Room, Female Vox Plate, Just a Touch, Large Room, Small Room, Snare Verb, Some Depth e Soundstage. Short é acrescentada aos mesmos registros, preservando as demais categorias. Fotos de Drum Room e Female Vox Plate confirmam os parâmetros já registrados. Novo Just a Touch = REV, decay 1,5 s, LF ×1,5, HF ×0,5, predelay 0 ms, Rear; Dry/Wet aproximadamente 30% do curso (estimativa visual, sem leitura numérica). Wet Solo e Noise aparecem ligados e continuam globais. Demais nomes listados mantêm os mapeamentos das referências anteriores.

### Referências da tag Reverb

Primeira lista visível confirma Blessers Barn, Carlos Caverns, Clean Space, Drum Room, Female Vox Plate, In da Club e Just a Touch. Todos já possuíam Reverb, sem novas instâncias ou mudanças nos parâmetros. Fotos detalhadas confirmam Blessers Barn = 1,8 s / LF ×1 / HF ×0,5 / predelay 20 ms / Rear; Carlos Caverns = 4,0 s / LF ×1,5 / HF ×0,5 / 40 ms / Rear; Clean Space = 3,3 s / LF ×0,5 / HF ×0,33 / 40 ms / Front. Lista ainda pode continuar abaixo da área visível; não foram removidos outros presets Reverb anteriormente mapeados.
