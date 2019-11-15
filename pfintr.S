/* pfintr.S - pfintr */

           .text
pferrcode: .long 0
           .globl  pfintr,pferrcode 
pfintr:
    popl pferrcode
    pushfl          
    cli    
    pusha        
    call pfint    
    popa        
    popfl         
    iret