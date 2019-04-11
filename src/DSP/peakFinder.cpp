#include <bits/stdc++.h>
using namespace std;


// returns a peak, position pair
vector<vector<int>> find_peaks(int arr[], int n)
{

     /* Declaring a vector to store the peak elements */
     std::vector<vector<int>> vect;

     /*Checking if first element of array is peak or not */
     if(arr[0]>arr[1]) vect.push_back({arr[0], 0});


     //checking rest of the elements

     for(int i=1;i<n-1;i++){
         if(arr[i]>arr[i-1] && arr[i]>arr[i+1]){
             vect.push_back({arr[i], i});
         }
     }


      /*Checking if last element of array is peak or not */
      if(arr[n-1]>arr[n-2]) vect.push_back({arr[n-1], n-1});
      return vect;

}
int main(){

    int n;

    cout<<"\nEnter the number of elements:\n";
    cin>>n;

    int arr[n];

    cout<<"\nEnter the elements: \n\n";
    for(int i=0;i<n;i++){
        cin>>arr[i];
    }

    vector<vector<int>> result=find_peaks(arr,n);

    cout<<"\nThe peak elements in the array are: \n";
    for(int i=0;i<result.size();i++){
         cout<<result[i][0]<<" at "<<result[i][1]<<"\n";
    }

    return 0;

}
