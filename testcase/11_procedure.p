program test(in, out, err);
var x, y, z : integer;

procedure xaddyz;
begin
    x := y+z
end;

procedure zmulxy;
begin
    z := x*y
end;

begin
    x := 3;
    y := 4;
    z := 5;
    xaddyz;
    zmulxy;

    printInt(x+y+z)
end.

