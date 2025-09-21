1. Change the root directory of the file system to the following structure.  
    <pre>
    /--+ counter (file)                                          /--+ input (dir)  
       |                                                            |   |            
       + subdir (dir)                     ----->                    |   +-- a (file)  
         |                                                          |   +-- b (file)  
         +-- subcounter (file)                                      |                 
                                                                    + output (dir)    
                                                                      |               
                                                                      +-- add (file)  
                                                                      +-- sub (file)
    </pre> 
2. Can set the values of a and b (range: 0–255) by using:  
   - echo number > /input/a
   - echo number > /input/b
3. Can obtain the results by:
   - cat /output/add → returns a + b
   - cat /output/sub → returns a – b


Execution process:
![image](https://github.com/20sharon/LinuxKernel/blob/main/HW5_myfs/Execution_process_01.png)
![image](https://github.com/20sharon/LinuxKernel/blob/main/HW5_myfs/Execution_process_02.png)
