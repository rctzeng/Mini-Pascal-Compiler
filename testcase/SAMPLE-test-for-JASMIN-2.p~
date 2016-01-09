PROGRAM foo(input, output, error) ;
   // variable declaraions
   var a, b, c, u: integer;
   var d: array [ 1 .. 10 ] of integer; 
   var e: array [ 1 .. 10 ] of real; 
   var g, h, x: real;
   var k: array [ 23 .. 57 ] of array [ 23 .. 57 ] of real;
   var l: array [ 23 .. 57 ] of array [ 23 .. 57 ] of integer;
   // procedure and function declarations
   procedure sort(a: array [ 1 .. 10 ] of integer);
      var b: integer; 
      begin
        c := b
      end;


   function  addition(a, b: integer) : integer;
     // var c: integer;  // local declaration
      begin
         addition := a + b   // this is the return value
      end;
	     function  ggyy(g, h: real) : real;
     // var c: integer;  // local declaration
      begin
         ggyy := g + h   // this is the return value
      end;  


	  
   begin
   
   //subprogram with parameter and return values
      c := addition(8,8);
	  writeln(c);
	  
	  c := addition(-8,-8);
	  writeln(c);
	  
	  x := ggyy(3.3,5.5);
	  writeln(x);
	  
	  
	//if then else
        b :=0;
		if b > a then a := 111 else a := 222;
		writeln(a);
		if b < a then a := 111 else a := 222;
		writeln(a);
		if b = a then a := 111 else a := 222;
		writeln(a);
        if b >= a then a := 111 else a := 222;
		writeln(a);
        if b <= a then a := 111 else a := 222;
		writeln(a);
        if b != 0 then a := 111 else a := 222;
        writeln(a);
	  
   
   
   //while loop
      a := 1;
      while a < 5 do
      begin
		 writeln(a);
         a   := a + 1 
      end;
	  
	  
      begin  // nested compound statement
         b := 1;
         while b < 11 do
         begin  // array initialization
            d[b] := b + b * (b + 3);  //assignment of 1-dimensional arrays int
            b := b +1
         end
        
      end;
	  e[3] := 9.99;				//assignment of 1-dimensional arrays real
      k[25][26] := 3.14;     // good  assignment of multi-dimensional arrays real
	  l[28][26] := 88;			// good  assignment of multi-dimensional arrays int
	  //k[25][126]  := 3.14;   // index out of bound
      //k[125][26]  := 3.14;   // index out of bound
      //k[125][126] := 3.14;   // index out of bound

	writeln(a);
	u := d[5];		//1-dimensional arrays int
	writeln(u);
	
	g := e[3];		//1-dimensional arrays real 9.99
	writeln(g);
		g := k[25][26]; // multi-dimensional arrays real 3.14
	writeln(g);
	
	u := l[28][26];	//multi-dimensional arrays int 88
	writeln(u);
	
	u := 9+9-10;    //8
	writeln(u);
	
	u := (9+9)/3+5*2-1+u; //23
	writeln(u)
	


	
   end.   // this is the end of the program
