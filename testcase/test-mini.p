PROGRAM Sort(input, output);
    CONST
        MaxElts = 50;
    TYPE 
        IntArrType = ARRAY [1..MaxElts] OF Integer;

    VAR
        i, j, tmp, size: integer;
        arr: IntArrType;

    PROCEDURE ReadArr(VAR size: Integer; VAR a: IntArrType);
        BEGIN
            size := 1;
            WHILE NOT eof DO BEGIN
                readln(a[size]);
                IF NOT eof THEN 
                    size := size + 1
            END
        END;

    PROCEDURE Quicksort(size: Integer; VAR arr: IntArrType);
        PROCEDURE QuicksortRecur(start, stop: integer);
            VAR
                m: integer;

                splitpt: integer;

            FUNCTION Split(start, stop: integer): integer;
                VAR
                    left, right: integer;  
                    pivot: integer;       

                PROCEDURE swap(VAR a, b: integer);
                    VAR
                        t: integer;
                    BEGIN
                        t := a;
                        a := b;
                        b := t
                    END;

                BEGIN 
                    pivot := arr[start];
                    left := start + 1;
                    right := stop;

                    WHILE left <= right DO BEGIN
                        WHILE (left <= stop) AND (arr[left] < pivot) DO
                            left := left + 1;
                        WHILE (right > start) AND (arr[right] >= pivot) DO
                            right := right - 1;
                        IF left < right THEN 
                            swap(arr[left], arr[right]);
                    END;

                    swap(arr[start], arr[right]);

                    Split := right
                END;

            BEGIN 
                IF start < stop THEN BEGIN
                    splitpt := Split(start, stop);
                    QuicksortRecur(start, splitpt-1);
                    QuicksortRecur(splitpt+1, stop);
                END
            END;
                    
        BEGIN 
            QuicksortRecur(1, size)
        END;

    BEGIN
        ReadArr(size, arr);

        Quicksort(size, arr);

        FOR i := 1 TO size DO
            writeln(arr[i])
    END.
