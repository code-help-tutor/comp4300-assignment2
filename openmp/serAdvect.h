WeChat: cstutorcs
QQ: 749389476
Email: tutorcs@163.com
// serial 2D advection solver module

// number of FLOPs to update a single element in the advection function 
extern const int advection_flops_per_element; 

// parameters needed for external advection solvers
extern const double Velx, Vely; //advection velocity
extern double dt;               //time for 1 step
extern double deltax, deltay;   //grid spacing       

// initializes the advection parameters for a global M x N field 
void init_advection_parameters(int M, int N);

// calculate 1D coefficients for the advection stencil
void calculate_and_update_coefficients(double v, double *cm1, double *c0, double *cp1);

//update 1 timestep for the local advection, without updating halos
//  the m x n row-major array (v,ldd) is updated from (u,ldu)
//  Assumes a halo of width 1 are around this array;
//  the corners of the halo are at u[-1,-1], u[-1,n], u[m,-1] and u[m,n]
void update_advection_field(int m, int n, double *u, int ldu, double *v, int ldv);

// initialize (non-halo elements) of a m x n local advection field (u,ldu)
//    local element [0,0] is element [M0,N0] in the global field
void init_advection_field(int M0, int N0, int m, int n, double *u, int ldu);

// sum errors in an m x n local advection field (u,ldu) after r timesteps 
//    local element [0,0] is element [M0,N0] in the global field 
double compute_error_advection_field(int r, int M0, int N0, int m, int n, double *u, int ldu);

//get abs max error in an m x n local advection field (u,ldu) after r timesteps
double compute_max_error_advection_field(int r, int M0, int N0, int m, int n, double *u, 
			 int ldu);

// print out the m x n local advection field (u,ldu) 
void print_advection_field(int rank, char *label, int m, int n, double *u, int ldu);

// copy m x n field (v, ldv) to ((u, ldu)
void copy_field(int m, int n, double *v, int ldv, double *u, int ldu);
