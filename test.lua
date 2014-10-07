
--print( mysin( 90 ), '\n' )

--print( mysin( 'sdfsdf' ), '\n')

function test( a )
    print(a, 'test!\n')
end

local t = {
     name =  "$$$$$$$$"
    ,value = "1"
    ,forward = {
         zero = 0
        ,one  = 1
        ,two  = 2
     }
    ,backward = {
          [0] = 'zero'
         ,[1] = 'one'
         ,[2] = 'two'
    }
    ,tg = globt
}

a = t;
a.name = 'a!!!!!!!!!'

print( t )
