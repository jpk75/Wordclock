/*******************************************************/
/* File: wordclock.scad                                */
/* Description: Parametric Wordclock                   */
/* Date: 2022-02-06                                    */
/* Author: _jpk                                        */
/* License: CCA - Non-Comercial - Share Alike          */
/*******************************************************/
quality_high = false;
quality_factor = 0.3;
generate_frame=true;
generate_inlet=false;
generate_back_cover = false;
generate_cable_guide = true;
generate_brigthness_sensor = true;
generate_diffusor = true;
generate_support = true;

wall_thickness_side = 1.9;  // wall thickness of side walls
wall_thickness_bottom = 1.5; // wall thickness of bottom
diffusor_thickness = 0.6;  // thickness of diffusor
support_thickness = 0.4;    // thickness of support

frame_width = 230;      // total width of word clock frame
frame_depth = 230;       // total depth of word clock frame
frame_height = 28;     // total height of word clock frame
mount_width_1 = 30;     // width 1 of mount
mount_width_2 = 20;     // width 2 of mount
mount_depth = 10;       // depth of mount
mount_height = 15;      // height of mount
mount_hole_d = 5.2;     // hole for screw
mount_offset_x = frame_width/2 - wall_thickness_side;
mount_offset_y = frame_depth/2 - wall_thickness_side;

cableguide_offset_x = -15;  // cable guide offset
cableguide_d = 6;           // diameter of cable guide tube

corner_offset_x=95;      // width of corners
corner_offset_y=95;       // depth of corners
corner_type=2;          // 0=circle, 1=rectangle, 2=tbd
corner_width = 5;       // width of corners (or for type 0: diameter
corner_depth=5;       // depth of corners

stripe_guide_depth=8.25;

base_width = frame_width-(2*wall_thickness_side) - 2;
base_depth = frame_depth-(2*wall_thickness_side) - 2;
base_height_1 = 2;
base_height_2 = 11;
base_height_3 = 2;
base_height_total = base_height_1 + base_height_2 + base_height_3;
base_led_hole_d = 9;
base_mount_tolerance = 0.2;

letter_font="Comic Sans MS:style=Bold";
letter_size=9.5;          // height of letter 
letter_dist=16.6667;       // distance between letters

clock_text="ESKISTAFÜNFZEHNZWANZIGDREIVIERTELVORFUNKNACHHALBAELFÜNFEINSDCWZWEIDREIAUJVIERSECHSNLACHTSIEBENZWÖLFZEHNEUNKUHR";
letter_per_row = 11;        // Number of letter per row

row_cnt = len(clock_text) / letter_per_row;
row_width = letter_per_row * letter_dist;
row_dist = row_width / row_cnt;
row_offset_x = (row_width - letter_dist)/ 2;
row_offset_y = (row_width - row_dist) / 2;

back_cover_width=base_width;
back_cover_depth=base_depth;
back_cover_height=frame_height-base_height_total;

brightness_hole_offset_x = 25;
brightness_hole_offset_y = frame_depth/2-15;
brightness_hole_d = 7;
WordClock();

// Module for creating a word clock frame
module WordClock(){
    echo(len(clock_text));
    echo(row_cnt, row_width, row_dist, row_offset_x, row_offset_y);

;    $fa = 0.5;
    $fs = quality_high ? quality_factor : 1;
    
    if (generate_frame)
        union(){
            color("BlueViolet",1){
                difference(){
                    frame(frame_width, frame_depth, frame_height);
                    corners();
                    
                    clock_text();
                }
            }
            if (generate_diffusor)
                color("White", 0.5){
                    translate([0,0,-(diffusor_thickness+frame_height)/2]) cube([frame_width, frame_depth,diffusor_thickness], center=true);
                }
            if (generate_support){
                
                support_offset_z = (frame_height - support_thickness)/2 + (generate_diffusor ? diffusor_thickness:0);
                translate([-frame_width/2,-frame_depth/2,-support_offset_z]) cylinder(support_thickness, d=20, center=true);
                translate([frame_width/2,-frame_depth/2,-support_offset_z]) cylinder(support_thickness, d=20, center=true);
                translate([frame_width/2,frame_depth/2,-support_offset_z]) cylinder(support_thickness, d=20, center=true);
                translate([-frame_width/2,frame_depth/2,-support_offset_z]) cylinder(support_thickness, d=20, center=true);
            }
        }
    
    
    if (generate_inlet){
        union(){
            base();
           if (generate_support){               
                support_offset_z = base_height_1 + support_thickness/2;
                translate([-base_width/2,-base_depth/2,-support_offset_z]) cylinder(support_thickness, d=20, center=true);
                translate([base_width/2,-base_depth/2,-support_offset_z]) cylinder(support_thickness, d=20, center=true);
                translate([base_width/2,base_depth/2,-support_offset_z]) cylinder(support_thickness, d=20, center=true);
                translate([-base_width/2,base_depth/2,-support_offset_z]) cylinder(support_thickness, d=20, center=true);
            }
        }
    }
    
    if (generate_back_cover){
        color("Green",1) 
            union(){
                back_cover();
                if (generate_support){               
                    support_offset_z = back_cover_height-base_height_1 - support_thickness/2;
                    translate([-base_width/2,-base_depth/2,support_offset_z]) cylinder(support_thickness, d=20, center=true);
                    translate([base_width/2,-base_depth/2,support_offset_z]) cylinder(support_thickness, d=20, center=true);
                    translate([base_width/2,base_depth/2,support_offset_z]) cylinder(support_thickness, d=20, center=true);
                    translate([-base_width/2,base_depth/2,support_offset_z]) cylinder(support_thickness, d=20, center=true);
                }
            }
        }
}

module frame(width=10, depth=10, height=10){
    difference(){
        cube([width, depth, height], center=true);
        translate([0,0,wall_thickness_bottom]){
            cube([width-2*wall_thickness_side, depth-2*wall_thickness_side, height], center=true);
        }
        
        if (generate_brigthness_sensor){
            d = brightness_hole_d;
            h = wall_thickness_bottom+0.2;
            translate([brightness_hole_offset_x, -brightness_hole_offset_y, -(frame_height-h)/2]) cylinder(h=h, r1=(d+wall_thickness_bottom)/2, r2=d/2, center=true);
        }

        if (generate_cable_guide){
            translate([cableguide_offset_x, frame_depth/2, frame_height/2]) 
                rotate([90,0,0]) 
                  cylinder(h=frame_depth/2, d=cableguide_d,center=true);

        }
    }
    mounts([0,-depth/2+wall_thickness_side,-frame_height/2], [0,0,0], h=mount_height);
    mounts([0,-depth/2+wall_thickness_side,-frame_height/2], [0,0,90], h=mount_height);
    mounts([0,-depth/2+wall_thickness_side,-frame_height/2], [0,0,180], h=mount_height);
    mounts([0,-depth/2+wall_thickness_side,-frame_height/2], [0,0,270], h=mount_height);
}

module mounts(offset=[0,0,0], rotation=[0,0,0], h=mount_height, hole=true, outline=0){
    rotate(rotation){
        translate(offset){
            translate([0,0,h/2])
            difference(){
                linear_extrude(height=h, center=true){
                    x1 = mount_width_1/2+outline;
                    x2 = mount_width_2/2+outline;
                    y1 = outline;
                    y2 = mount_depth+outline;
                    polygon([[-x1,y1],[x1,y1],[x2,y2],[-x2,y2]]);
                }
                if (hole) translate([0, mount_depth/2, (h+1)/2]) cylinder(h+1, d=mount_hole_d, center=true);
            }
        }
    }
}

module corners(){
    if (corner_type == 1){
        // corner type rectangle
        translate([-corner_offset_x, -corner_offset_y, 0]) cube([corner_width, corner_depth, frame_height], center=true);
        translate([-corner_offset_x, corner_offset_y, 0])  cube([corner_width, corner_depth, frame_height], center=true);
        translate([corner_offset_x, corner_offset_y, 0])   cube([corner_width, corner_depth, frame_height], center=true);
        translate([corner_offset_x, -corner_offset_y, 0])  cube([corner_width, corner_depth, frame_height], center=true);
    }
    else if (corner_type == 2){
        // tbd: complex type
        corner_complex([-corner_offset_x, -corner_offset_y, 0]);
        corner_complex([-corner_offset_x, -corner_offset_y, 0], [0, 0, 90]);
        corner_complex([-corner_offset_x, -corner_offset_y, 0], [0, 0, 180]);
        corner_complex([-corner_offset_x, -corner_offset_y, 0], [0, 0, 270]);
    }
    else
    {
        // default type: circle
        translate([-corner_offset_x, -corner_offset_y, 0]) cylinder(frame_height, d=corner_width, center=true);
        translate([-corner_offset_x, corner_offset_y, 0])  cylinder(frame_height, d=corner_width, center=true);
        translate([corner_offset_x, corner_offset_y, 0])   cylinder(frame_height, d=corner_width, center=true);
        translate([corner_offset_x, -corner_offset_y, 0])  cylinder(frame_height, d=corner_width, center=true);
    }
}

module corner_complex(offset=[0,0,0], rotation=[0,0,0]){
    rotate(rotation){
        translate(offset){
            difference(){
                cube([corner_width, corner_depth, frame_height], center=true);
                translate([2,2,0]) cube([corner_width, corner_depth, frame_height], center=true);
            }
            translate([-(corner_width/2)+1, (corner_depth/2)]) cylinder(frame_height, d=2, center=true);
            translate([(corner_width/2), -(corner_depth/2)+1]) cylinder(frame_height, d=2, center=true);
        }
    }
}

module clock_text(){
    text_len = len(clock_text);
    mirror([0,1,0])
    translate([0,0,-frame_height]) {
        for(i=[0:text_len-1]){
            r = floor(i / letter_per_row);
            c = i% letter_per_row;
            echo("row:", r, "col:", c);
            
            letter(clock_text[i], letter_size, -row_offset_x+(c*letter_dist), row_offset_y-(r*row_dist));
        }
    }
}

module letter(letter, size=10, x=0, y=0, bridge_w=1){
    translate([x,y,0]){
        difference(){
            linear_extrude(height=frame_height, convexity=4)
                text(letter,
                     size=size,
                     font=letter_font,
                     halign="center",
                     valign="center");
            if (!generate_diffusor){
                if(letter=="A" || letter=="Ä")
                    translate([0,-size/2,0])
                    cube([bridge_w, size/2, frame_height], center=false);
                else if (letter=="B"){
                    translate([-size/2,-2,0])
                        cube([size/2, bridge_w, frame_height]);
                    translate([-size/2,2,0])
                        cube([size/2, bridge_w, frame_height]);
                }
                else if(letter=="D" || letter == "O" || letter=="Ö"){
                     translate([-size/2,-bridge_w,0])
                        cube([size/2, bridge_w, frame_height]);
               }
               else if(letter=="R"){
                     translate([-size/2,2,0])
                        cube([size/2, bridge_w, frame_height]);
               }
            }
        }
    }
}

module corner_cover(size=[10,10, 10], wall_thickness = 1){
    rotate([0,0,45]) difference(){
        cube(size, center=true);
        cube(size - [wall_thickness, 2*wall_thickness, -2], center=true);
    }
}

module stripe_guide(){
    w = row_width - letter_dist;
    for(i=[-(row_offset_y+row_dist/2):row_dist:row_offset_y+row_dist/2]){
        translate([0,i,base_height_3]){
            cube([w, stripe_guide_depth, base_height_3], center=true);
            translate([-w/2, 0, 0]) cylinder(base_height_3, d=stripe_guide_depth, center=true);
            translate([w/2, 0, 0]) cylinder(base_height_3, d=stripe_guide_depth, center=true);
        }
    }
    
    translate([0,0, base_height_3]) difference(){
        cube([base_width, base_depth, base_height_3], center=true);
        cube([base_width-wall_thickness_side, base_depth-wall_thickness_side, base_height_3+2], center=true);
    }
}

module base(){
    base_edge = 7;
    difference(){
        
        translate([0,0, -base_height_1 - base_height_1/2]) {
            cube([base_width, base_depth, base_height_1], center=true);

            offset_x = (row_width + (1*wall_thickness_bottom))/2;
            offset_y = (row_width + (1*wall_thickness_bottom))/2;
            
 //         stripe_guide();
          
          translate([0,0,-(base_height_2)]) linear_extrude(base_height_2) polygon([
                [-(offset_x-base_edge), (offset_y)],
                [(offset_x-base_edge), (offset_y)],
                [(offset_x), (offset_y-base_edge)],
                [(offset_x), -(offset_y-base_edge)],
                [(offset_x-base_edge), -(offset_y)],
                [-(offset_x-base_edge), -(offset_y)],
                [-(offset_x), -(offset_y-base_edge)],
                [-(offset_x), (offset_y-base_edge)]
                ]); 
             
            corner_cover_offset_x = corner_offset_x-0.5;
            corner_cover_offset_y = corner_offset_y-0.5;
            translate([-corner_cover_offset_x, -corner_cover_offset_y,-(base_height_2/2)]) 
                corner_cover([letter_dist,letter_dist,base_height_2], wall_thickness_bottom);
            translate([corner_cover_offset_x, -corner_cover_offset_y,-(base_height_2/2)]) 
                corner_cover([letter_dist,letter_dist,base_height_2], wall_thickness_bottom);
            translate([corner_cover_offset_x, corner_cover_offset_y,-(base_height_2/2)]) 
                corner_cover([letter_dist,letter_dist,base_height_2], wall_thickness_bottom);
            translate([-corner_cover_offset_x, corner_cover_offset_y,-(base_height_2/2)]) 
                corner_cover([letter_dist,letter_dist,base_height_2], wall_thickness_bottom);
            
        }
        mnt_offset_h = -frame_height/2 + wall_thickness_bottom;
        mounts([0,-mount_offset_x,mnt_offset_h], [0,0,0], hole=false, outline=base_mount_tolerance);
        mounts([0,-mount_offset_x,mnt_offset_h], [0,0,90], hole=false, outline=base_mount_tolerance);
        mounts([0,-mount_offset_x,mnt_offset_h], [0,0,180], hole=false, outline=base_mount_tolerance);
        mounts([0,-mount_offset_x,mnt_offset_h], [0,0,270], hole=false, outline=base_mount_tolerance);
        translate([-corner_offset_x, -corner_offset_x, 0]) cylinder(2*frame_height, d=base_led_hole_d, center=true);
        translate([corner_offset_x, -corner_offset_x, 0]) cylinder(2*frame_height, d=base_led_hole_d, center=true);
        translate([corner_offset_x, corner_offset_x, 0]) cylinder(2*frame_height, d=base_led_hole_d, center=true);
        translate([-corner_offset_x, corner_offset_x, 0]) cylinder(2*frame_height, d=base_led_hole_d, center=true);
        
        text_len = len(clock_text);
        
        cut_x = letter_dist - wall_thickness_bottom;
        cut_y = row_dist - wall_thickness_bottom;
        cut_edge = base_edge - (wall_thickness_bottom * tan(22.5));
      
        for(i=[0:text_len-1]){
            r = floor(i / letter_per_row);
            c = i% letter_per_row;
            translate([-row_offset_x+(c*letter_dist), row_offset_y-(r*row_dist),0]) cylinder(2*frame_height, d=base_led_hole_d, center=true);
            translate([-row_offset_x+(c*letter_dist), row_offset_y-(r*row_dist),-(base_height_2/2+base_height_1+base_height_3)]) {
                bFirstRow = (r==0);
                bLastRow = (r==(row_cnt-1));
                bFirstCol = (c==0);
                bLastCol = (c==(letter_per_row-1));
                
                if ((bFirstRow||bLastRow) && (bFirstCol||bLastCol)){
                    angle_z = 0; //bFirstRow ? 0 : 180;
                    mir_x = bFirstRow ? (bLastCol ? 1 :0) : (bLastCol ? 1 : 0);
                    mir_y = bFirstRow ? (bLastCol ? 0 :0) : (bLastCol ? 1 : 1);
                    
                    translate([0,0, -(base_height_2/2)])mirror([mir_x,0,0]) mirror([0,mir_y,0])rotate([0,0, angle_z])
                        linear_extrude(base_height_2)
                            polygon([[-(cut_x/2-cut_edge), cut_y/2],[cut_x/2,cut_y/2],[cut_x/2,-cut_y/2],[-cut_x/2, -cut_y/2],[-cut_x/2, cut_y/2-cut_edge]]);
                }
                else {
                    cube([cut_x, cut_y, base_height_2], center=true);
                }
            }
        }
        
        // cable hole brightness sensor
        if (generate_brigthness_sensor){
            translate([2*brightness_hole_offset_x, -base_depth/2,0]) hull(){
                translate([0, 2.5,0]) cylinder(h=10, d=5, center=true);
                translate([0,-2.5,0]) cylinder(h=10, d=5, center=true);
            }
        }
    }
}

module back_cover(){
    difference(){
        union(){
            back_cover_base();
            if (generate_cable_guide){
                translate([cableguide_offset_x, (back_cover_depth/4+9), back_cover_height-base_height_1]) 
                    rotate([90,0,0]) 
                        difference(){
                            d = cableguide_d+1;
                            p = d + 4;
                            h = back_cover_depth/2 - 18;
                            union(){
                                cylinder(h=h, d=d,center=true);
                                translate([0,0,back_cover_depth/4-p]) cylinder(h=p, r2=p, r1=d/2, center=true);
                            }
                            translate([0,p,0]) cube([2*p,2*p,h+20], center=true);
                        }
            }
        }
        if (generate_cable_guide){
            d = cableguide_d;
            p = d;
            translate([cableguide_offset_x, (back_cover_depth/4+1), back_cover_height-base_height_1]) 
                rotate([90,0,0]){ 
                    cylinder(h=back_cover_depth/2, d=d,center=true);
                    translate([0,0,back_cover_depth/4-15]) cylinder(h=p, r2=p, r1=d/2, center=true);
                }
        }
    }
}

module back_cover_base(){
    w = 2* wall_thickness_side;
    translate([0,0,back_cover_height/2-base_height_1]){
        difference(){
            union(){
                difference(){
                    cube([back_cover_width, back_cover_depth, back_cover_height], center=true);
                    translate([0,0,-wall_thickness_bottom]) cube([back_cover_width-w, back_cover_depth-w, back_cover_height], center=true);
                }
                cube([60, 30, back_cover_height-1], center = true);
                mounts([0,-mount_offset_x,-back_cover_height/2-wall_thickness_bottom], [0,0,0], h=back_cover_height, hole=false, outline=wall_thickness_side);
                mounts([0,-mount_offset_x,-back_cover_height/2-wall_thickness_bottom], [0,0,90], h=back_cover_height, hole=false, outline=wall_thickness_side);
                mounts([0,-mount_offset_x,-back_cover_height/2-wall_thickness_bottom], [0,0,180], h=back_cover_height, hole=false, outline=wall_thickness_side);
                mounts([0,-mount_offset_x,-back_cover_height/2-wall_thickness_bottom], [0,0,270], h=back_cover_height, hole=false, outline=wall_thickness_side);

            }
            translate ([0,0,wall_thickness_bottom]) cube([60-w, 30-w, back_cover_height], center = true);
            translate([-10,0,0]) rotate([0,90,0]) cylinder(60, d=8, center = true);
            
            back_cover_mount([-(back_cover_width/2-20), -(back_cover_depth/2-20), 1]);
            back_cover_mount([ (back_cover_width/2-20), -(back_cover_depth/2-20), 1]);

            mnt_cut_h = wall_thickness_bottom + 0.1;
            mounts([0,-mount_offset_x,-back_cover_height/2- mnt_cut_h], [0,0,0], h=mnt_cut_h, hole=false, outline=0.1);
            mounts([0,-mount_offset_x,-back_cover_height/2-mnt_cut_h], [0,0,90], h=mnt_cut_h, hole=false, outline=0.1);
            mounts([0,-mount_offset_x,-back_cover_height/2-mnt_cut_h], [0,0,180], h=mnt_cut_h, hole=false, outline=0.1);
            mounts([0,-mount_offset_x,-back_cover_height/2-mnt_cut_h], [0,0,270], h=mnt_cut_h, hole=false, outline=0.1);
            
        translate([0, (mount_offset_y - mount_depth/2), 0]) cylinder(back_cover_height+1, d=3.2, center=true);
        translate([0, -(mount_offset_y - mount_depth/2), 0]) cylinder(back_cover_height+1, d=3.2, center=true);
        translate([-(mount_offset_x - mount_depth/2), 0, 0]) cylinder(back_cover_height+1, d=3.2, center=true);
        translate([(mount_offset_x - mount_depth/2), 0, 0]) cylinder(back_cover_height+1, d=3.2, center=true);
            
        translate([0, (mount_offset_y - mount_depth/2), 5]) cylinder(back_cover_height, d=5.8, center=true);
        translate([0, -(mount_offset_y - mount_depth/2), 5]) cylinder(back_cover_height, d=5.8, center=true);
        translate([-(mount_offset_x - mount_depth/2), 0, 5]) cylinder(back_cover_height, d=5.8, center=true);
        translate([(mount_offset_x - mount_depth/2), 0, 5]) cylinder(back_cover_height, d=5.8, center=true);
        }
    }
}

module back_cover_mount(offset=[0,0,0]){
    translate(offset) 
        hull(){
            cylinder(back_cover_height, d=3.5,center=true);
            translate([0, -3.5,0]) cylinder(back_cover_height, d=3.5,center=true);
        }
    translate(offset)translate([0,3.5,0])cylinder(back_cover_height, d=8, center=true);
}