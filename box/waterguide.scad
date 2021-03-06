
// Outher Diameter of the pot
outerDiamPot=118;
// Wall diameter of the pot
wallDiamPot=2.4;
// Diameter 
diamGuide=12;
// Distance between water holes
waterHoleDistance=9;
// Offset of cubes
offset=15;
//InnerTiameterTube;
tubeInnerDiam=8;
// Size water holes
sproudSize=1.6;

$fn = $preview ? 12 : 72;

difference() {
    union() {
        pipe(length=outerDiamPot+offset,i=diamGuide-3.2,o=diamGuide,cap=[2,2],cut=false);
        rotate([0,90,0]) pipe(o=diamGuide,cap=[2,0],length=10,cut=false);
        translate([10,0,0]) rotate([0,90,0]) pipe(o=tubeInnerDiam,i=5,cap=[0,0],length=14,cut=false);
        // Block to hold pot
        blockOffset=offset-8/2;
        translate([0,-diamGuide/2,blockOffset]) 
            cube([15,diamGuide,8]);
        translate([0,-diamGuide/2,outerDiamPot-wallDiamPot+blockOffset]) 
            cube([15,diamGuide,8]);
    }
    union() {
        //translate([-250,0,-250]) cube([500,20,500]);
        pipe(length=outerDiamPot+15,i=diamGuide-4,o=diamGuide+1,cap=[2,2],cut=true);
        rotate([0,90,0]) pipe(cap=[2,0],i=6,o=10,length=10,cut=true);
       
        // Water holdes
        offsetToCenterHoles=(outerDiamPot-8*waterHoleDistance)/2;
        translate([0,0,offset+offsetToCenterHoles]) {
            mirror([0,1,0]) waterHoles();
            waterHoles();
        }
        
        translate([diamGuide/2,0,offset]) potRing();
    }
}

module waterHoles() {
    for ( i = [0 : 4] ){
        color([0,1,0]) translate([0, 0, i*waterHoleDistance*2])
        rotate([90,00,15]) cylinder(d=sproudSize,h=20);
    }
    for ( i = [0 : 3] ){
        color([0,0,1]) translate([0, 0, i*waterHoleDistance*2+waterHoleDistance])
        rotate([90,00,00]) cylinder(d=sproudSize,h=20);
    }
}

module potRing() {
    height=20;
    translate([height/2,0,outerDiamPot/2-wallDiamPot/2]) 
    rotate([0,90,0]) difference() {
        dInner=outerDiamPot-(wallDiamPot+0.2)*2;
        cylinder(d1=outerDiamPot,d2=outerDiamPot-3,h=height,center=true);
        cylinder(d1=dInner,d2=dInner-3,h=21,center=true);
    }
}

module pipe(i=5,o=10,length=50,cap=[0,0], cut=false) {
    translate([0,0,length/2]) {
        difference() {
            union() {                
                if (cut==false) {
                    cylinder(d=o,h=length,center=true);
                    if (cap[0]>0) {
                        cap(o,-1,cap[0],false);
                    }   
                    if (cap[1]>0) {
                        cap(o,1,cap[1],false);
                    }    
                }
            }
            union() {
                cylinder(d=i,h=length,center=true);
                if (cap[0]>0) {
                    cap(i,-1,cap[0],true);
                }   
                if (cap[1]>0) {
                    cap(i,1,cap[1],true);
                }   
            }
        } 
    }
    

    module cap(diam,pos,capStyle,diff) {
        translate([0,0,pos*length/2]) {
            if (capStyle==1 && diff==false) {
                translate([0,0,pos*1]) cylinder(d=diam,h=2,center=true);
            }
            if (capStyle==2) sphere(d=diam);
        }
    }
 
}