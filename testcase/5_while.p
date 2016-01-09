program test(in, out, err);
var a, b, c, d, e : integer;
begin
    a := 3;
    b := 5;
    c := 7;
    d := 100;
    while(b > a) do
    begin
        b := b-1
    end;
    while(c > b) do
    begin
        c := c-1
    end;
    while(d > c) do
    begin
        d := d-1
    end;

    printInt(a+b+c+d)
end.

