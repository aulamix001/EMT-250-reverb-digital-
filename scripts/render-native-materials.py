"""Reproducible orthographic CAD-style material renders; run with Blender --background --python this-file."""
import bpy
import math
from pathlib import Path
from mathutils import Vector
root=Path(__file__).resolve().parent.parent/'native'/'assets'

def setup(width,height,ortho,camera):
    bpy.ops.wm.read_factory_settings(use_empty=True)
    scene=bpy.context.scene
    scene.render.engine='CYCLES';scene.cycles.samples=64;scene.cycles.use_denoising=False
    scene.render.threads_mode='FIXED';scene.render.threads=4
    scene.render.resolution_x=width;scene.render.resolution_y=height;scene.render.resolution_percentage=100
    scene.render.film_transparent=True
    scene.render.image_settings.file_format='PNG';scene.render.image_settings.color_mode='RGBA'
    scene.view_settings.view_transform='Standard';scene.view_settings.look='None'
    scene.world=bpy.data.worlds.new('Studio ambience');scene.world.use_nodes=True
    scene.world.node_tree.nodes['Background'].inputs['Color'].default_value=(.3,.3,.28,1)
    scene.world.node_tree.nodes['Background'].inputs['Strength'].default_value=.35
    bpy.ops.object.camera_add(location=camera);cam=bpy.context.object;cam.rotation_euler=(Vector((0,0,0))-cam.location).to_track_quat('-Z','Y').to_euler();cam.data.type='ORTHO';cam.data.ortho_scale=ortho;scene.camera=cam
    for name,location,power,size in [('Left softbox',(-3,-3,5),180,3),('Narrow reflection',(-2,1,4),85,.6),('Right fill',(3,-1,4),30,3)]:
        bpy.ops.object.light_add(type='AREA',location=location);lamp=bpy.context.object;lamp.name=name;lamp.data.energy=power;lamp.data.shape='RECTANGLE';lamp.data.size=size;lamp.data.size_y=4;lamp.rotation_euler=(Vector((0,0,0))-lamp.location).to_track_quat('-Z','Y').to_euler()
    return scene

def material(name,color,roughness=.3,metal=.05,grain=True):
    m=bpy.data.materials.new(name);m.use_nodes=True;n=m.node_tree.nodes;bs=n.get('Principled BSDF')
    bs.inputs['Base Color'].default_value=(*color,1);bs.inputs['Roughness'].default_value=roughness;bs.inputs['Metallic'].default_value=metal;bs.inputs['Coat Weight'].default_value=.18
    if grain:
        noise=n.new('ShaderNodeTexNoise');noise.inputs['Scale'].default_value=210;noise.inputs['Detail'].default_value=2
        bump=n.new('ShaderNodeBump');bump.inputs['Strength'].default_value=.07;bump.inputs['Distance'].default_value=.008
        m.node_tree.links.new(noise.outputs['Fac'],bump.inputs['Height']);m.node_tree.links.new(bump.outputs['Normal'],bs.inputs['Normal'])
    return m

scene=setup(1024,512,3.2,(0,-2.7,8))
bpy.ops.object.light_add(type='AREA',location=(0,-3,1.5));lamp=bpy.context.object;lamp.data.energy=150;lamp.data.size=2;lamp.rotation_euler=(Vector((0,0,0))-lamp.location).to_track_quat('-Z','Y').to_euler()
for x,ivory in [(-.85,True),(.85,False)]:
    side=material('Ivory resin' if ivory else 'Red anodized lacquer',(.52,.52,.45) if ivory else (.32,.023,.026),.31 if ivory else .29)
    bottom=material('Underside ivory' if ivory else 'Underside red',(.75,.75,.64) if ivory else (.25,.025,.028),.38)
    top=material('Dark top',(.22,.22,.19) if ivory else (.18,.012,.015),.4)
    bpy.ops.mesh.primitive_cylinder_add(vertices=128,radius=.5,depth=.6,location=(x,0,0),rotation=(math.pi/2,0,0))
    cap=bpy.context.object;cap.name='Ivory slider cap' if ivory else 'Red slider cap'
    for m in [side,bottom,top]:cap.data.materials.append(m)
    for face in cap.data.polygons:
        face.material_index=1 if face.normal.z>.5 else 2 if face.normal.z<-.5 else 0
        face.use_smooth=abs(face.normal.z)<.5
    bevel=cap.modifiers.new('Molded rim','BEVEL');bevel.width=.018;bevel.segments=3
    cap.modifiers.new('Weighted normals','WEIGHTED_NORMAL')
scene.render.filepath=str(root/'fader-caps.png');bpy.ops.render.render(write_still=True)

scene=setup(256,768,2.3,(0,-.8,8))
verts=[];faces=[];rings=241;sides=96
for j in range(rings):
    y=-1+2*j/(rings-1);r=.27+.045*math.cos((y+1)*math.pi*10)
    for i in range(sides):
        angle=i*2*math.pi/sides;verts.append((r*math.cos(angle),y,r*math.sin(angle)))
for j in range(rings-1):
    for i in range(sides):faces.append((j*sides+i,j*sides+(i+1)%sides,(j+1)*sides+(i+1)%sides,(j+1)*sides+i))
mesh=bpy.data.meshes.new('Corrugated rubber mesh');mesh.from_pydata(verts,[],faces);mesh.update()
obj=bpy.data.objects.new('Rubber accordion sleeve',mesh);bpy.context.collection.objects.link(obj);obj.data.materials.append(material('Dark rubber',(.035,.038,.032),.38,.03,False))
for face in mesh.polygons:face.use_smooth=True
scene.render.filepath=str(root/'bellows.png');bpy.ops.render.render(write_still=True)
