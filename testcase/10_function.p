program test(in, out, err);
var x, y, z : integer;

function add(a, b : integer) : integer;
begin
    add := a+b
end;

function mul(a, b : integer) : integer;
begin
    mul := a*b
end;

begin
    x := add(3, 4);
    y := mul(x, x);
    z := add(y, mul(x, 1));

    printInt(x+y+z)
end.

