$fn=60;

// Diameter of probe
probeDiam=6;        
// Spacing between probe
probeSpacing=16;
// THickness of wall
wall=2.0;

// Length of probe (Visual only)
probeLength=120;

// Diameter of screw
screwDiam=3;
// Height of screw head
screwHead=2.5;
// Diameter of screw head
screwHeadDiam=5.8;

///Offset probe from center of casing
probeEndOffset=5;

// Do not change below
wall2=wall*2;
probeSpacingD2=probeSpacing/2;
dim=[probeSpacing + probeDiam + wall2,probeEndOffset*2+4*2+wall2,probeDiam+wall2];

echo("Casing dimention ");
echo(dim);


soilProbe(true);
translate([dim.x+5,0,0]) soilProbe(false);

module soilProbe(bottom=true) {
    translate([0,0,dim.z/2])
    union() {
        difference() {
            union() {
                cube(dim, center=true);
                
            }
            union() {
                // Cut in half
                translate([0,0,dim.z/2]) cube(dim, center=true);

                // Room for Wire
                translate([0,probeEndOffset+2,0]) {
                    cube([probeSpacing+probeDiam/2,4,probeDiam], center=true);
                    if (bottom) {
                        translate([3,0,0]) cylinder(d=2.5,h=dim.z,center=true);
                        translate([-3,0,0]) cylinder(d=2.5,h=dim.z,center=true);
                    }
                }
                
                // Probe
                translate([probeSpacingD2,probeEndOffset,0]) probe();
                translate([-probeSpacingD2,probeEndOffset,0]) probe();      
                
                // screw Portion
                screw();
            }
        }

        %translate([probeSpacingD2,probeEndOffset,0]) probe();
        %translate([-probeSpacingD2,probeEndOffset,0]) probe();
    }
}


module screw() {
    cylinder(d=screwDiam,h=dim.z,center=true);
    translate([0,0,-dim.z/2+screwHead/2]) cylinder(d=screwHeadDiam,h=screwHead,center=true);
}    


module probe() {
    pointL=probeDiam*2.5;
    color([0.5,1,0.5]) rotate([90,0,0]) {
        cylinder(d=probeDiam,h=probeLength-pointL);
        translate([0,0,probeLength-pointL]) cylinder(d2=0, d1=probeDiam, h=probeDiam*2.5);
    }

}

