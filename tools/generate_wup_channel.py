#!/usr/bin/env python3
from __future__ import annotations
import argparse, shutil, struct, subprocess
from pathlib import Path
from xml.etree import ElementTree as ET

TITLE_ID='00050000464E3355'
GROUP_ID='0000464E'
TITLE_VERSION_HEX='0001'
TITLE_VERSION_DEC='1'
PRODUCT_CODE='WUP-N-FN3U'
RPX='fnaf3-wiiu.rpx'
LONG_NAME="Five Nights at Freddy's 3 - Wii U Edition"
SHORT_NAME='FNaF3 Wii U'
PUBLISHER='Eitan1414 and contributors'


def add(root, tag, typ, length, value=''):
    node=ET.SubElement(root, tag, {'type':typ, 'length':str(length)})
    node.text=value
    return node


def write_xml(path, root):
    path.parent.mkdir(parents=True, exist_ok=True)
    ET.indent(root, space='  ')
    ET.ElementTree(root).write(path, encoding='utf-8', xml_declaration=True)


def app_xml():
    root=ET.Element('app', {'type':'complex','access':'777'})
    for item in [
        ('version','unsignedInt',4,'1'),('os_version','hexBinary',8,'000500101000400A'),
        ('title_id','hexBinary',8,TITLE_ID),('title_version','hexBinary',2,TITLE_VERSION_HEX),
        ('sdk_version','unsignedInt',4,'21201'),('app_type','hexBinary',4,'80000000'),
        ('group_id','hexBinary',4,GROUP_ID),('os_mask','hexBinary',32,'0'*64)]:
        add(root,*item)
    return root


def cos_xml():
    root=ET.Element('app', {'type':'complex','access':'777'})
    for item in [
        ('version','unsignedInt',4,'1'),('cmdFlags','unsignedInt',4,'0'),
        ('argstr','string',4096,RPX),('avail_size','hexBinary',4,'00000000'),
        ('codegen_size','hexBinary',4,'02000000'),('codegen_core','hexBinary',4,'80000001'),
        ('max_size','hexBinary',4,'40000000'),('max_codesize','hexBinary',4,'01000000')]:
        add(root,*item)
    permissions=ET.SubElement(root,'permissions',{'type':'complex'})
    for i,group in enumerate((1,3,9,12,11,13,14,15,16,17,18,19,20,21,22)):
        p=ET.SubElement(permissions,f'p{i}',{'type':'complex'})
        add(p,'group','unsignedInt',4,str(group)); add(p,'mask','hexBinary',8,'FFFFFFFFFFFFFFFF')
    for core in range(3): add(root,f'default_stack{core}_size','hexBinary',4,'00000000')
    for core in range(3): add(root,f'default_redzone{core}_size','hexBinary',4,'00000000')
    for core in range(3): add(root,f'exception_stack{core}_size','hexBinary',4,'00001000')
    return root


def meta_xml():
    root=ET.Element('menu', {'type':'complex','access':'777'})
    fields=[
      ('version','unsignedInt',4,'1'),('product_code','string',32,PRODUCT_CODE),('content_platform','string',32,'WUP'),
      ('company_code','string',8,'0001'),('mastering_date','string',32,''),('logo_type','unsignedInt',4,'0'),
      ('app_launch_type','hexBinary',4,'00000000'),('invisible_flag','hexBinary',4,'00000000'),
      ('no_managed_flag','hexBinary',4,'00000000'),('no_event_log','hexBinary',4,'00000000'),
      ('no_icon_database','hexBinary',4,'00000000'),('launching_flag','hexBinary',4,'00000004'),
      ('install_flag','hexBinary',4,'00000000'),('closing_msg','unsignedInt',4,'1'),
      ('title_version','unsignedInt',4,TITLE_VERSION_DEC),('title_id','hexBinary',8,TITLE_ID),
      ('group_id','hexBinary',4,GROUP_ID),('boss_id','hexBinary',8,'0000000000000000'),
      ('os_version','hexBinary',8,'000500101000400A'),('app_size','hexBinary',8,'0000000000000000'),
      ('common_save_size','hexBinary',8,'0000000000000000'),('account_save_size','hexBinary',8,'0000000000000000'),
      ('common_boss_size','hexBinary',8,'0000000000000000'),('account_boss_size','hexBinary',8,'0000000000000000'),
      ('save_no_rollback','unsignedInt',4,'0'),('join_game_id','hexBinary',4,'00000000'),
      ('join_game_mode_mask','hexBinary',8,'0000000000000000'),('bg_daemon_enable','unsignedInt',4,'0'),
      ('olv_accesskey','unsignedInt',4,'0'),('wood_tin','unsignedInt',4,'0'),('e_manual','unsignedInt',4,'0'),
      ('e_manual_version','unsignedInt',4,'0'),('region','hexBinary',4,'FFFFFFFF')]
    for item in fields: add(root,*item)
    ratings=('cero','esrb','bbfc','usk','pegi_gen','pegi_fin','pegi_prt','pegi_bbfc','cob','grb','cgsrr','oflc','reserved0','reserved1','reserved2','reserved3')
    for name in ratings:
        add(root,'pc_'+name,'unsignedInt',4,'192' if name in ('bbfc','pegi_fin','reserved0','reserved1','reserved2','reserved3') else '128')
    for tag,val in [('ext_dev_nunchaku','0'),('ext_dev_classic','0'),('ext_dev_urcc','1'),('ext_dev_board','0'),('ext_dev_usb_keyboard','0'),('ext_dev_etc','0')]: add(root,tag,'unsignedInt',4,val)
    add(root,'ext_dev_etc_name','string',512,'')
    for tag,val in [('eula_version','0'),('drc_use','1'),('network_use','0'),('online_account_use','0'),('direct_boot','0')]: add(root,tag,'unsignedInt',4,val)
    for i in range(8): add(root,f'reserved_flag{i}','hexBinary',4,'00010001' if i==0 else ('00000003' if i==6 else '00000000'))
    langs=('ja','en','fr','de','it','es','zhs','ko','nl','pt','ru','zht')
    for lang in langs: add(root,f'longname_{lang}','string',512,LONG_NAME)
    for lang in langs: add(root,f'shortname_{lang}','string',256,SHORT_NAME)
    for lang in langs: add(root,f'publisher_{lang}','string',256,PUBLISHER)
    for i in range(32): add(root,f'add_on_unique_id{i}','hexBinary',4,'00000000')
    return root


def convert_tga(source, destination, width, height, pixel_format):
    destination.parent.mkdir(parents=True, exist_ok=True)
    subprocess.run([
        'ffmpeg','-hide_banner','-loglevel','error','-y','-i',str(source),
        '-vf',f'scale={width}:{height}:flags=lanczos,format={pixel_format}',
        '-frames:v','1','-c:v','targa',str(destination)], check=True)


def validate_tga(path, width, height, bpp):
    header=path.read_bytes()[:18]
    if len(header)!=18: raise RuntimeError(f'{path}: invalid TGA')
    got=(*struct.unpack_from('<HH',header,12),header[16])
    if got!=(width,height,bpp): raise RuntimeError(f'{path.name}: {got} != {(width,height,bpp)}')


def main():
    root=Path(__file__).resolve().parents[1]
    artwork=root/'.wiiu-artwork'
    p=argparse.ArgumentParser()
    p.add_argument('--rpx',type=Path,default=root/RPX)
    p.add_argument('--output',type=Path,default=root/'build-wup'/'channel')
    p.add_argument('--icon',type=Path,default=artwork/'icon.png')
    p.add_argument('--tv',type=Path,default=artwork/'boot-tv.png')
    p.add_argument('--drc',type=Path,default=artwork/'boot-drc.png')
    a=p.parse_args()

    if not all(source.is_file() for source in (a.icon,a.tv,a.drc)):
        prep=root/'tools'/'prepare_wiiu_artwork.sh'
        if prep.is_file():
            subprocess.run(['bash',str(prep)],check=True)

    for source in (a.rpx,a.icon,a.tv,a.drc):
        if not source.is_file(): raise SystemExit(f'Missing required file: {source}')
    out=a.output.resolve()
    if out.exists(): shutil.rmtree(out)
    (out/'code').mkdir(parents=True); (out/'content').mkdir(); (out/'meta').mkdir()
    write_xml(out/'code'/'app.xml',app_xml()); write_xml(out/'code'/'cos.xml',cos_xml()); write_xml(out/'meta'/'meta.xml',meta_xml())
    shutil.copy2(a.rpx,out/'code'/RPX)
    convert_tga(a.icon,out/'meta'/'iconTex.tga',128,128,'bgra')
    convert_tga(a.tv,out/'meta'/'bootTvTex.tga',1280,720,'bgr24')
    convert_tga(a.drc,out/'meta'/'bootDrcTex.tga',854,480,'bgr24')
    validate_tga(out/'meta'/'iconTex.tga',128,128,32)
    validate_tga(out/'meta'/'bootTvTex.tga',1280,720,24)
    validate_tga(out/'meta'/'bootDrcTex.tga',854,480,24)
    print(f'Generated Wii U install channel in {out}')
    print(f'Title ID: {TITLE_ID}')

if __name__=='__main__': main()
