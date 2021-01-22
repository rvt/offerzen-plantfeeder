$fn=60;

boxX=55;
boxY=80;
boxZ=25;
wall=2;
screwSize=2.5;
diamMotorWire=3;

locationESP=[[5,7,0],[0,0,0]];
locationHydro=[[20+15,boxY-9,0], [0,0,90]];
locationTIPC31C=[[boxX-11,9,0], [0,0,0]];
locationKroon=[[boxX-11,boxY/2+5,0], [0,0,0]];
locationHoleMotor=[[boxX-5,boxY-20,boxZ-15], [0,90,0]];


// BOX&Stuff
difference() {
    union() {
        box();
        translate([-wall,-wall,-wall]) boxCornerTab(wall=wall,bottom=wall,innerWidth=boxX,innerLength=boxY,innerHeight=boxZ);
        TR(locationHydro) hygro(heightPcb=5, cut=false);
        TR(locationTIPC31C) color([1,0,0]) TIPC31C(heightPcb=5);
        TR(locationKroon) color([0,1,0]) kroon(heightPcb=5);
        TR(locationESP) esp8266_az(cut=false, heightPcb=5);
    }
    union() {
        translate([-wall,-wall,-wall]) boxCornerTab(wall=wall,bottom=wall,innerWidth=boxX,innerLength=boxY,innerHeight=boxZ,cut=true);
        TR(locationESP) esp8266_az(cut=true, heightPcb=5);
        TR(locationHydro) hygro(heightPcb=5, cut=true);
        TR(locationHoleMotor) cylinder(d=3,h=10);
    }
}

// Little ring
translate([0,boxY+10,-wall]) {
    difference() {
        union() {
            cylinder(d=3.2+4*0.4,h=1.5);
            cylinder(d=8,h=0.5);
        }
        cylinder(d=3.2,h=4);
    }
}

// LID
translate([60,0,0]) {
    difference() {
        lid();
            translate([-wall,-wall,-boxZ]) 
            boxCornerTab(wall=wall,bottom=wall,innerWidth=boxX,innerLength=boxY,innerHeight=boxZ,cut=true);
    }
}

module lid() {
    boxZ=wall;
    translate([boxX/2,boxY/2,boxZ/2-wall])
    cube([boxX+wall*2, boxY+wall*2, boxZ], center=true);
}


// Translate and Rotate
module TR(in) {
    translate(in[0]) rotate(in[1]) children();
}

module box() {
    translate([boxX/2,boxY/2,boxZ/2-wall])
        difference() {        
            cube([boxX+wall*2, boxY+wall*2, boxZ], center=true);
            translate([0,0,wall]) cube([boxX, boxY, boxZ], center=true);
        }
}

module esp8266_az(heightPcb=7, cut=false) {
    pos=[
        [0,0],
        [25,0],
        [0,52],
        [25,52]
    ];
    board=[[-3,-3],[31,57]];
    if (cut) {
        translate([0,0,heightPcb]+[0,0,-1.5]) 
        translate(board[0] + [0-11/2 + board[1].x/2,-17,0]) 
        translate([0,0,-3.2]) cube([11,17,7.4]);
    } else {
        %difference() {
            translate([0,0,heightPcb]) translate(board[0]) linear_extrude(height=1.5) square(board[1]);
            screwHoles(pos, heightPcb=heightPcb, diff=true);
        }
        screwHoles(pos, heightPcb=heightPcb, diff=false);
    }
}


module screwHoles(holes, heightPcb=5, diff=false) {
    for (idx = [ 0 : len(holes) - 1 ] ) {
        translate(holes[idx]) {
            difference() {
                union() {
                    if (diff==false)
                        cylinder(d=5,h=heightPcb-0.1,center=false);
                }
                cylinder(d=screwSize,h=heightPcb+5,center=false);
            }
        }
    }
}

module TIPC31C(heightPcb=3) {    
    b=[[10.4,16,heightPcb], [10.4/2,3,0]];        
    block(b);
}

module kroon(heightPcb=5) {
    b=[[17.4,21,heightPcb], [17.4/2,21/2,0]];        
    block(b);
}

module hygro(heightPcb=5, cut=false) {
    circuitThick = 1.5;
    board=[
        [-14.2/2,-31/2],
        [14.2,31],
    ];
    
    b=[[14.2,9,heightPcb], [14.2/2,9/2,0]];
    if (cut) {
        translate([-3,-15-31/2,-3/2 + heightPcb + 5.5]) cube([6,15,3]);
    } else {
        %translate([0,0,heightPcb]) 
            translate(board[0]) 
            linear_extrude(height=1.5) square(board[1]);
        translate([0,7,0]) block(b);
    }
}

module boxCornerTab(wall=2.0,bottom=2,innerWidth=30,innerLength=40,innerHeight=30,cut=false) {
    cornerHeight=20;
    b=[
        [[wall,wall,0],[0,0,0]],
        [[innerWidth+wall,wall,0],[0,0,90]],
        [[innerWidth+wall,innerLength+wall,0],[0,0,180]],
        [[wall,innerLength+wall,0],[0,0,270]]
    ];
    
    for (idx = [ 0 : len(b) - 1 ] ) {
        TR(b[idx]) corner(cut);
    }

    module corner(cut=false) {
        translate([0,0,innerHeight-cornerHeight])
        color([0.8,0.8,0.2]) 
        if (cut) {
            translate([3,3,cornerHeight/2]) cylinder(d=screwSize+0.5,h=innerHeight);
        } else {
            hull() {
                translate([0,0,cornerHeight]) cube([6,6,0.001]);
                translate([0,0,cornerHeight/2]) cube([4,4,0.001]);
                cube([0.1,0.1,0.001]);
            }        
        }
    }    
} 

module block(block) {
    translate([-block[0].x/2,-block[0].y/2]) {
        translate([-0.8,0,0]) cube([0.8, block[0].y, block[0].z+1]);
        translate([block[0].x,0,0]) cube([0.8, block[0].y, block[0].z+1]);    
        difference() {
            cube(block[0]);
            translate(block[1]) cylinder(d=screwSize,h=block[0].z+2);
        }
    }
}
