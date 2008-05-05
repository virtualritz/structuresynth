/*
This version of the Binary Kite 
demonstrates how different template render types can be set

Try rendering it using the sunflowcolored render template.
*/

set maxobjects 8000
10 * { y 1 } 10 * { z 1 }  1 * { a 0.8  sat 0.9 ry -3 rx -3  rz -3 } r1 
set background #aaa
set template white
{ z -5 s 1000 1000 0.1 } box 

rule r1   {
  { x 1  ry 4 } r1
  xbox
}

rule r1   {
{ x 1  ry -4  } r1
xbox
}

rule r1   {
{ x 1  rz -8  s 0.95 } r1
xbox
}

rule r1   {
{ x 1  rz 8  s 0.95   } r1
xbox
}



rule r2 maxdepth 36 {
{ ry 1  ry -13 x  1.2 b 0.99 h 12  } r2 
xbox
}

rule xbox {
  set template white
  { b 0.7  color #000   a 0.9  }  box
}

rule xbox {
  set template black
 { b 0.7  color #ddd  a 0.9    } box
}