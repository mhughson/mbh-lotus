#!/usr/bin/python3

import sys
import json
import os
import subprocess

#try:
#    sys.argv[1]
#except (IndexError, NameError):
#	print ("\n!!ERROR: expected json filename of map as an argument!!")
    #todo: go back to input once work flow is established.
#	jsonmap_filename = "small_map.json" # input("Enter name of map file now: ")
#else:	
#	jsonmap_filename = sys.argv[1]

#jsonmap_file = open(jsonmap_filename, 'r')
#jsonmap_data = json.load(jsonmap_file)
#print(jsonmap_data["editorsettings"]["export"]["format"])

#mapname = os.path.basename(jsonmap_filename)
#mapname = os.path.splitext(mapname)[0]

# required for when this is run from visual studio, where Working directory will be /game/ not /game/MAPS/
#os.chdir("GAME/MAPS/")
os.chdir("MAPS/")
print("CWD: " + os.getcwd())

# NOTE: NEW_TILESET_CHANGE_REQUIRED
tileset_to_index_mapping = {
    "nametable_temp.json" : "0",
    "nametable_overworld.json" : "1",
}

tileset_to_music_index_mapping = {
    "nametable_temp.json" : "0",
    "nametable_overworld.json" : "0",
}

tileset_to_bgpal_index_mapping = {
    "nametable_temp.json" : "0",
    "nametable_overworld.json" : "1",
}

tileset_to_sprpal_index_mapping = {
    "nametable_temp.json" : "0",
    "nametable_overworld.json" : "0",
}

object_type_name_mapping = {
    "player_spawn_point" : "0",
    "trans_point" : "1",
    "trans_edge" : "2",
}

flag_names = [
    "flag0_enable_grav_flip", 
    "flag1_enable_room_flip",
    "flag2_enable_seamless_travel",
    "flag3_enable_auto_movement",
]

def CombineFlags(props):

    flgs = 0
    for count, name in enumerate(flag_names):
        for j in props:
            if name == j["name"]:
                if j["value"] == True:
                    flgs |= 1 << count

    return flgs

num_rooms = 0
num_worlds = 0
num_levels = 1
num_special_levels = 1

def GenerateHeader(file_name, world_files):

    global num_worlds
    global num_levels
    global num_special_levels

    room_name_list = [ ]

    print('Generating ' + file_name + '.h')

    newfile = open('../' + file_name +'.h', 'w')  # warning, this will overwrite old file !!!!!!!!!!!!!!!!!!!!!

    newfile.write("// auto-generated by generate_maps_header.py\n")


    # *.world files are passed in, in the order that they should appear
    # in game.
    for wf in world_files:

        print(wf.upper())

        world_file = open(wf, 'r')
        world_data = json.load(world_file)

        # create a temporary copy of the data for this world.
        sorted_obj = dict(world_data)

        # sort the list of map files based on x and y position from 
        # top-left, to bottom right. y is the primary sort,
        # and then x will be used to sort objects with the same Y value.
        sorted_obj['maps'] = sorted(world_data['maps'], key=lambda x : (x['y'], x['x']), reverse=False)

        # create a list of just the file names.
        # we don't care about any of the other data at this point.
        map_list = [element['fileName'] for element in sorted_obj['maps']]

        num_levels = 1

        for f in map_list:
            global num_rooms
            print("processing (" + str(num_rooms) + "): " + f)

            num_rooms += 1

            jsonmap_file = open(f, 'r')
            jsonmap_data = json.load(jsonmap_file)

            mapname = os.path.basename(f)
            mapname = os.path.splitext(mapname)[0]

            room_name_list.append(mapname)
            
            layers = jsonmap_data["layers"]

            width = jsonmap_data["width"]
            height = jsonmap_data["height"]

            # Ensure that the width of the map (in tiles) is a power of 2.
            # The runtime code depends on this as an optimization.
            if (width & (width-1) != 0) or width == 0:
                raise NameError("Width of map (in tiles) must be a power of 2, but it is " + str(width))

            # First count how many dynamic objects are on the map.
            layer = [x for x in layers if x["name"].lower()=="dynamics"][0] # layers[1] 
            counter = 0
            for d in layer["data"]:
                if d > 0:
                    # index and tile id
                    counter += 2

            # Could now many dynamic objects there are, multiplied by the number of bytes
            # written out for each one, and then +4 for the trailing 0xff
            layer = [x for x in layers if x["name"].lower()=="objects"][0] # layers[1] 
            counter += (len(layer["objects"]) * 4) + 4
                

            # The map array will be 960 for the BG + the index and tile id for each
            # dynamic object. The extra +2 is for ending characters 0xff,0xff.
            # +3 more for palette overrides and tileset.
            # +1 for the bit flags
            # +1 special room type
            # +1 music track
            # +2 world-level
            # +2 width/height
            newfile.write("const unsigned char " + mapname + "[" + str((width * height) + counter + 3 + 2 + 1 + 1 + 1 + 2 + 2) + "] = \n{\n")

            # TODO: This has a pretty nasty assumption that properties come in order, and have either 100%
            # defined, or none.
            if len(jsonmap_data["tilesets"]) > 1:
                raise NameError("Multiple Tilesets Found in " + f)
                
            BG_Palette_Override = tileset_to_bgpal_index_mapping[jsonmap_data["tilesets"][0]["source"]]
            SPR_Palette_Override = tileset_to_sprpal_index_mapping[jsonmap_data["tilesets"][0]["source"]]
            Special_Type = 0
            Music_Override = tileset_to_music_index_mapping[jsonmap_data["tilesets"][0]["source"]]
            Bit_Flags = 0

            if "properties" in jsonmap_data:
                for prop in jsonmap_data["properties"]:
                    if prop["name"] == "BG_Palette_Override":
                        BG_Palette_Override = prop["value"]
                    elif prop["name"] == "SPR_Palette_Override":
                        SPR_Palette_Override = prop["value"]
                    elif prop["name"] == "Special_Type":
                        Special_Type = prop["value"]
                    elif prop["name"] == "Music_Override":
                        Music_Override = prop["value"]
                
                Bit_Flags = CombineFlags(jsonmap_data["properties"])

            newfile.write("\n//BG PAL:\n" + str(BG_Palette_Override) + ",\n")
            newfile.write("\n//SPR PAL:\n" + str(SPR_Palette_Override) + ",\n")
            
            newfile.write("\n//TILESET:\n" + tileset_to_index_mapping[jsonmap_data["tilesets"][0]["source"]] + ",\n")
            
            newfile.write("\n//BIT FLAGS:\n" + str(Bit_Flags) + ",\n")

            newfile.write("\n//SPECIAL MAP TYPE:\n" + str(Special_Type) + ",\n")

            newfile.write("\n//MUSIC TRACK:\n" + str(Music_Override) + ",\n")

            if (Bit_Flags & 1<<2):
                newfile.write("\n//CHAPTER:\n" + str(0) + ",\n")
                newfile.write("\n//FLOOR:\n" + str(num_special_levels) + ",\n")
                num_special_levels += 1
            else:
                newfile.write("\n//CHAPTER:\n" + str(num_worlds) + ",\n")
                newfile.write("\n//FLOOR:\n" + str(num_levels) + ",\n")
                num_levels += 1

            newfile.write("\n//WIDTH:\n" + str(width) + ",\n")
            newfile.write("\n//HEIGHT:\n" + str(height) + ",\n")

            # BG Layer.
            layer = [x for x in layers if x["name"].lower()=="background"][0] # layers[1] 
            
            counter = 0
            # assumes order of layers is background, then dynamics.
            if layer["type"] == "tilelayer":
                newfile.write("\n//Background:\n")
                for d in layer["data"]:
                    display = str(d - 1)
                    display = display.rjust(2, " ")
                    newfile.write(display + ", ")
                    counter+=1
                    if counter == 16:
                        newfile.write("\n")
                        counter = 0   

            newfile.write("\n//Dynamics:\n")
            counter = 0
            # Dynamics layer
            layer = [x for x in layers if x["name"].lower()=="dynamics"][0] # layers[1] 
            for d in layer["data"]:
                if d > 0:
                    # index, id
                    newfile.write(str(counter) + ", " + str(d - 1) + ", ")
                counter += 1
            # Signals the end of the Dynamics data.
            newfile.write("0xff, 0xff, \n")

            newfile.write("\n//Objects: (type, tile_x, tile_y, payload) \n")
            counter = 0
            # Objects layer
            layer = [x for x in layers if x["name"].lower()=="objects"][0] # layers[1] 
            for objs in layer["objects"]:
                payload = 0
                if "properties" in objs and len(objs["properties"]) > 0:
                    payload = objs["properties"][0]["value"]
                newfile.write(
                    object_type_name_mapping[objs["type"]] + ", " + 
                    str(int(objs["x"] / 16)) + ", " + 
                    str(int(objs["y"] / 16)) + ", " +
                    str(payload) + ",\n") #warning: assumes first property is payload
                counter += 1
            # Signals the end of the Object data.
            newfile.write("0xff, 0xff, 0xff, 0xff \n};\n\n")            
        
        num_worlds += 1
        
    # delete that last comma, back it up	
    #z = newfile.tell()
    #z = z - 3
    #newfile.seek(z)

    newfile.write("const unsigned char (* const rooms_" + file_name + "[" + str(len(room_name_list)) +"]) =\n{\n")

    for room in room_name_list:
        newfile.write(room + ",\n")

    newfile.write("};")

    newfile.close


# MAP LIST A (BANK 4)
#"world_gym.world" 
#GenerateHeader("maps_a", [ "world_prologue.world", "world_castle_basic.world", "world_labyrinth.world", "world_gym.world" ])
GenerateHeader("maps_a", [ "world_gym.world" ])
#GenerateHeader("maps_a", [ "world_custom.world"])
room_split_b = num_rooms

# MAP LIST B (BANK 3)
#GenerateHeader("maps_b", [ "world_gravity.world", "world_mirror.world", "world_flip.world", "world_key.world" ])
#GenerateHeader("maps_b", [ ])
room_split_c = num_rooms

# MAP LIST C (BANK 2 - Partial)
#GenerateHeader("maps_c", [ "world_team.world", "world_boss.world" ])
#GenerateHeader("maps_b", [ ])

newfile = open('../map_defs.h', 'w')  # warning, this will overwrite old file !!!!!!!!!!!!!!!!!!!!!

newfile.write("// auto-generated by generate_maps_header.py\n")

newfile.write("#ifndef BCF202CC_2317_4F36_8DD6_9F5039E154C6\n#define BCF202CC_2317_4F36_8DD6_9F5039E154C6\n\n")

newfile.write("#define NUM_ROOMS " + str(num_rooms) + "\n")
newfile.write("#define ROOM_SPLIT_INDEX_B " + str(room_split_b) + "\n")
newfile.write("#define ROOM_SPLIT_INDEX_C " + str(room_split_c) + "\n")

newfile.write("\n#endif /* BCF202CC_2317_4F36_8DD6_9F5039E154C6 */\n")
newfile.close

print("\n**GENERATE MAPS HEADER COMPLETE**\n")