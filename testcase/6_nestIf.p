program test(in, out, err);
var a, b, c, d, e : integer;
begin
    a := 3;
    b := a*3;
    if(b > 7) then
    begin
        c := a+b-2;
        if(c < 5) then
        begin
            d := c+a-b
        end
        else
        begin
            d := c-a+b
        end
    end
    else
    begin
        c := a*b/2;
        if(c > 5) then
            d := 6
        else
            d := 7
    end;

    e := 7;

    printInt(a*b+c/d-e)
end.

