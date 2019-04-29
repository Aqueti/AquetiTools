#include "atl/polyfit.tcc"
#include <iostream>

int main()
{
    std::vector<double> r_undistorted;
    std::vector<double> r_distorted;
    //create the undistorted array
    for(double x = 0; x <= 1; x+=0.01){
        r_undistorted.push_back(x);
    }

    //Have the user enter A, B, C, D
    std::string A, B, C, D;
    std::cout << "A: ";
    std::cin >> A;
    std::cout << "B: ";
    std::cin >> B;
    std::cout << "C: ";
    std::cin >> C;
    std::cout << "D: ";
    std::cin >> D;

    //Create the distorted array 
    for(double r : r_undistorted){
        double newR = std::stod(A) * std::pow(r,4) + std::stod(B) * std::pow(r,3) + std::stod(C) * std::pow(r,2) + std::stod(D) * r;
        r_distorted.push_back(newR);
    }

    //Polyfit the distorted to the undistorted
    std::vector<double> coeffs = mathalgo::polyfit(r_distorted, r_undistorted,4);
    
    for(double coef : coeffs){
        std::cout << coef << std::endl;
    }
    return 0;

}
