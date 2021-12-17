program hello
    implicit none
    include 'mpif.h'
    integer err,rank,size
    call MPI_INIT(err)
    call MPI_COMM_RANK(MPI_COMM_WORLD,rank,err)
    call MPI_COMM_SIZE(MPI_COMM_WORLD,size,err)
    print *, 'I am ', rank, ' of ', size
    call MPI_FINALIZE(err)
end program hello
