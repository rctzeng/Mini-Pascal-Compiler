PROGRAM foo(input, output, error) ;

   var a, b : integer;
   
   // test recursive functions
   function  sum(a: integer) : integer;
      begin
         if a <= 0 then sum := 0
         else sum := a + sum(a-1)
         // sum is the return value
      end;
   
   begin  
      a := sum(10);   // the result is 55.
      b := sum(-10)   // the result is 0. 
   end.   // this is the end of the program
