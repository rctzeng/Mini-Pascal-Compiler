PROGRAM foo(input, output, error) ;
   // this test checks various operators

   var a, b, c: integer;
   var d, e: array [ 1 .. 10 ] of integer; 

   begin
      b := a + 93;  // simple expression
      b := a - 93;  // simple expression
      b := a * 93;  // simple expression
      b := a / 93;  // simple expression
      b := 3 + 93 * 5 + b * (23-15*6/9);  // complex expression
      
      a := 1;   // test relation operators
      if b > a then a := a * 2 else a := a * 3;
      if b < a then a := a * 5 else a := a * 7;
      if b = a then a := a * 11 else a := a * 13;
      if b >= a then a := a * 17 else a := a * 19;
      if b <= a then a := a * 23 else a := a * 29;
      if b != 0 then a := a * 31 else a := a * 37;
      if not (b > a) then a := a * 41 else a := a * 43
      
   end.   // this is the end of the program
