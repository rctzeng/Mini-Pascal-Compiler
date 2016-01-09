program test(in, out, err);
var a, b, c, d, e : integer;
begin
    a := 3;
    b := a*3;
    if(b > 50) then
    begin
        c := a+b-2
    end
    else
    begin
        c := b*a-7
    end;

    if(c > 5) then
    begin
        d := c+a-b
    end
    else
    begin
        d := c-a+b
    end;

    e := 7;

    printInt(a*b+c/d-e)
end.

