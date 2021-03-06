/**
 * @file radarunit.h
 * @author  Peter Mikula <mikula.ptr@gmail.com>
 * @version 1.0
 * @brief Class for processing raw data and their storage until they are graphically represented
 *
 * @section DESCRIPTION
 *
 * The radarUnit class is the target where all data are stored after they are processed. The objects
 * based on this class are stored transparently in list placed in main window object. We can consider
 * them as the virtual interpretation of real radars. Each radar unit has own identification and will
 * be processing the only data that belongs to it. Each radar unit is also independently on the others
 * making its own protocol about how many targets are visible, how are they organized, ... The final
 * data are stored in vectors. Also color assignments are done by the private methods and they are stored
 * in needed order in vectors of same size as vectors for coordinate storage.
 */

#ifndef RADARUNIT_H
#define RADARUNIT_H

#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <float.h>
#include <vector>
#include <QDebug>
#include <QThread>
#include <QList>

#include "uwbsettings.h"
#include "stddefs.h"
#include "rawdata.h"
#include "mtt_pure.h"

using namespace std;

class radarUnit
{
public:
    /**
     * @brief This construtor will ensure correct allocation for all data needed when program running
     * @param[in] radarId Is the radar identificator.
     * @param[in] x_pos Specifies the x position of radar unit in relation to operator coordinate system.
     * @param[in] y_pos Specifies the y position of radar unit in relation to operator coordinate system.
     * @param[in] rot_Angle Specifies the radar's coordinate system rotation in relation to operator coordinate system.
     * @param[in] enabled Specifies if radar unit should be considered during data fusion. Default is false. If automatic creation of unit by application is used, default value is specified.
     */
    radarUnit(int radarId, double x_pos = .0, double y_pos = .0, double rot_Angle = .0, bool enable = false);
    ~radarUnit();

    /**
     * @brief This function will read data from the container and apply MTT processing functions.
     * @param[in] data Container with all data obtained by reciever and retrieved from stack.
     * @param[in] enableMTT If is set to true, radar will internally use its own MTT algorithm to process data.
     * @return If the data processing was successful, the return value is true and the data in radarUnit are considered as updated.
     */
    bool processNewData(class rawData * data, bool enableMTT=false);

    /**
     * @brief This function returns the number of targets from latest data.
     * @return The return value is the number of targets.
     */
    int getNumberOfTargetsLast(void);

    /**
     * @brief Provides fast access to pointer of array with coordinates from latest iteration.
     * @return The return value is pointer to the array where all latest coordinates are stored.
     */
    float * getCoordinatesLast(void);

    /**
     * @brief Provides fast access to pointer of array with coordinates from iteration specified by index.
     * @param[in] index Is the depth of recursion from which we would like to obtain the number of targets.
     * @return The return value is the number of targets from specified iteration.
     */
    int getNumberOfTargetsAt(int index);

    /**
     * @brief Provides fast access to pointer of array with coordinates directly from iteration specified by index.
     * @param[in] index Is the depth of recursion from which we would like to obtain coordinates.
     * @return The return value is the pointer to array where all coordinates are stored.
     */
    float * getCoordinatesAt(int index);

    /**
     * @brief Returns the currently set x-position of radar unit in relation to operator unit. If is equal to zero (default value), probably was not set.
     * @return The return value is the x-position of radar unit.
     */
    double getXpos(void) { return xpos; }

    /**
     * @brief Sets new y position of radar unit in relation to operator coordinate system.
     * @param[in] angle Specifies new y position.
     */
    void setXpos(double x) { xpos = x; }

    /**
     * @brief Returns the currently set y-position of radar unit in relation to operator unit. If is equal to zero (default value), probably was not set.
     * @return The return value is the y-position of radar unit.
     */
    double getYpos(void) { return ypos; }

    /**
     * @brief Sets new y position of radar unit in relation to operator coordinate system.
     * @param[in] angle Specifies new y position.
     */
    void setYpos(double y) { ypos = y; }

    /**
     * @brief Returns the currently set rotation angle of radar unit in relation to operator unit. If is equal to zero (default value), probably was not set.
     * @return The return value is the rotation angle of radar unit.
     */
    double getRotAngle(void) { return rotAngle; }

    /**
     * @brief Sets new rotation angle of radar unit in relation to operator coordinate system.
     * @param[in] angle Specifies new rotation angle.
     */
    void setRotAngle(double angle) { rotAngle = angle; }

    /**
     * @brief Function returns the boolean value which specifies if the radar unit is allowed by user.
     * @return The return value is enabled state.
     */
    bool isEnabled(void) { return enabled; }

    /**
     * @brief This function will set the new state for radar unit.
     * @param[in] enable Is the new radar unit state.
     */
    void setEnabled(bool enable) { enabled = enable; }

    /**
     * @brief This function will take coordinates passed as parameters and calculates the transformation into operator coordinate system.
     * @param[in] x The input x position of target to transform.
     * @param[in] y The input y position of target to transform.
     *
     * This function will calculate coordinates transformated into operator's coordinate system and saves them into 'tempX' adn 'tempY'
     * variables. These can be accessed by another public functions.
     */
    void doTransformation(float x, float y);

    /**
     * @brief Returns the last result of target's x-position transformation.
     * @return The return value is lastly calculated x position.
     */
    float getTransformatedX(void) { return tempX; }

    /**
     * @brief Returns the last result of target's y-position transformation.
     * @return The return value is lastly calculated y position.
     */
    float getTransformatedY(void) { return tempY; }

    /**
     * @brief Function takes the array and checks for maximum targets allowed. If array is big, length > MAX_N*2 macro are deleted and array is rescaled. If length < MAX_N*2, rest is zeroed.
     * @param[in] array Is pointer to object holding the array that is required to process.
     */
    void zeroEmptyPositions(rawData * array);

    /**
     * @brief This function will delete and again create MTT object, so default values are set. This is necessary if starting new reciever or resuming after pause.
     */
    void resetMTT(void);


private:
    /* FOR TEST PURPOSES - TEST OF MTT_PURE LIBRARY */
    mtt_pure * mtt_p;

    int radar_id; ///< Main radar identificator
    bool enabled; ///< Specifies if the radar is enabled/disabled by user. If set to false, this unit should not be considered during data fusion. This value is always set to false if the unit was created by application automatically.
    unsigned int max_recursion; ///< The maximum data handling at one time dataList

    QList<rawData * > * dataList; ///< Handles all recieved data with MTT applied. Maximum number is specified in max_recursion

    double xpos; ///< Handles the x position of radar's coordinate system in relation to operator. If no value is specified, default value is 0.
    double ypos; ///< Handles the y position of radar's coordinate system in relation to operator. If no value is specified, default value is 0.
    double rotAngle; ///< Handles the rotation angle of radar's coordinate system in relation to operator.  If no value is specified, default value is 0.

    float tempX; ///< Stores the lastly transformated x coordinate.
    float tempY; ///< Stores the lastly transformated y coordinate.

    /*------------------------------------BASIC MATRIX WORKAROUND START-----------------------------------------------*/

    template <typename TYPE> // adds new row to matrix filled with values in array
    void addRow(vector < vector <TYPE> * > * matrix, TYPE * array, int array_size)
    {
        matrix->push_back(new vector<TYPE>);

        int elements_in_array = array_size/sizeof(TYPE);
        // now we can iterate over all elements in array
        for(int i=0; i<elements_in_array; i++)
            matrix->back()->push_back(array[i]);
    }

    /*template <typename TYPE2> // useable only in console
    void printMatrix(vector <vector <TYPE2> * > * matrix)
    {
        for(int row = 0; row < matrix->size(); row++)
        {
            for(int column = 0; column < matrix->at(row)->size(); column++)
                cout << matrix->at(row)->at(column) << "\t";

            cout << endl;
        }
    }*/

    template <typename TYPE3>
    TYPE3 getVal(int row, int column, vector <vector <TYPE3> * > * matrix)
    {
        return matrix->at(row-1)->at(column-1);
    }

    template <typename TYPE4>
    vector <vector <TYPE4> * > * productMatrix(vector <vector <TYPE4> * > * A, vector <vector <TYPE4> * > * B)
    {
        vector <vector <TYPE4> * > * resultMatrix = new vector <vector <TYPE4> * >;

        if(!this->checkMatrix(A)) return NULL;
        if(!this->checkMatrix(B)) return NULL;

        // now when we know that matrix are correct, we check for product condition
        if(A->at(0)->size()!=B->size()) return NULL;

        for(int nrows = 0; nrows < A->size(); nrows++)
        {
            resultMatrix->push_back(new vector <TYPE4>);
            for(int ncolumns = 0; ncolumns < B->at(0)->size(); ncolumns++)
            {
                TYPE4 value = 0;
                for(int i = 0; i < B->size(); i++)
                    value += A->at(nrows)->at(i)*B->at(i)->at(ncolumns);

                resultMatrix->back()->push_back(value);
            }
        }

        return resultMatrix;
    }

    template <typename TYPE5> // check if matrix is correct: if each row has the same number of elements
    bool checkMatrix(vector <vector <TYPE5> * > * matrix)
    {
        int count = matrix->at(0)->size();

        for(int i = 1; i < matrix->size(); i++)
            if(matrix->at(i)->size() != count) return false;

        return true;
    }

    template <typename TYPE6> // creates matrix with one row filled with values
    vector <vector <TYPE6> * > * createRowMatrix(TYPE6 * array, int array_size)
    {
        vector <vector <TYPE6> * > * new_vector = new vector <vector <TYPE6> * >;
        new_vector->push_back(new vector<TYPE6>);

        int number_of_elements = array_size/sizeof(TYPE6);

        for(int i = 0; i < number_of_elements; i++)
            new_vector->back()->push_back(array[i]);

        return new_vector;
    }

    template <typename TYPE7> // creates matrix with one column filled with values
    vector <vector <TYPE7> * > * createColumnMatrix(TYPE7 * array, int array_size)
    {
        int number_of_elements = array_size/sizeof(TYPE7);
        vector <vector <TYPE7> * > * new_vector = new vector <vector <TYPE7> * >;

        for(int i = 0; i < number_of_elements; i++)
        {
             new_vector->push_back(new vector <TYPE7>);
             new_vector->back()->push_back(array[i]);
        }

        return new_vector;
    }

    template <typename TYPE8> // sums two matrixes
    vector <vector <TYPE8> * > * sumMatrix(vector <vector <TYPE8> * > * A, vector <vector <TYPE8> * > * B)
    {
        if(!this->checkMatrix(A) || !this->checkMatrix(B)) return NULL;

        // check for basic sum condition
        if((A->size() != B->size()) || (A->at(0)->size() != B->at(0)->size())) return NULL;

        vector <vector <TYPE8> * > * sum = new vector <vector <TYPE8> * >;

        for(int rows = 0; rows < A->size(); rows++)
        {
            sum->push_back(new vector <TYPE8>);

            for(int columns = 0; columns < A->at(0)->size(); columns++)
            {
                sum->back()->push_back(A->at(rows)->at(columns) + B->at(rows)->at(columns));
            }
        }

        return sum;
    }

    template <typename TYPE9>
    vector <vector <TYPE9> * > * productMatrixWithNumber(vector <vector <TYPE9> * > * matrix, TYPE9 number)
    {
        if(!checkMatrix(matrix)) return NULL;

        vector <vector <TYPE9> * > * new_matrix = new vector <vector <TYPE9> * >;

        for(int row = 0; row < matrix->size(); row++)
        {
            new_matrix->push_back(new vector <TYPE9>);

            for(int column = 0; column < matrix->at(0)->size(); column++)
            {
                new_matrix->back()->push_back(matrix->at(row)->at(column)*number);
            }
        }

        return new_matrix;
    }

    template <typename TYPE10>
    void deleteMatrix(vector <vector <TYPE10> * > * matrix)
    {
        for(int counter = 0; counter < matrix->size(); counter++)
        {
            delete matrix->at(counter);
        }

        delete matrix;
    }

    template <typename TYPE11>
    double laplaceDeterminant(vector <vector <TYPE11> * > * matrix, bool checkMatrix = true)
    {
        if(checkMatrix)
        {
            if(!this->checkMatrix(matrix)) return -1.0;

            // checking for square matrix
            if(matrix->at(0)->size()!=matrix->size()) return -1.0;
        }


        // recursive calculating until we are at the end of firs line

        if(matrix->size() == 1) return this->getVal(1, 1, matrix);
        else {
            double a1j = 0.0;
            double result = 0.0;
            vector <vector <TYPE11> * > * cutMatrix;

            for(int j = 1; j <= matrix->at(0)->size(); j++)
            {
                a1j = this->getVal(1, j, matrix);

                cutMatrix = this->deleteRow(1, this->deleteColumn(j, matrix)); // matrix without first row and j-th column

                result = result + a1j*(pow((-1.0), 1+j))*this->laplaceDeterminant(cutMatrix, false);

                this->deleteMatrix(cutMatrix);
            }

            return result;
        }

        return -1;
    }

    template <typename TYPE12>
    vector <vector <TYPE12> * > * deleteRow(int row, vector <vector <TYPE12> * > * matrix)
    {
         vector <vector <TYPE12> * > * newMatrix = this->copyMatrix(matrix);

         /*for(int i = 0; i < matrix->size(); i++)
         {
             newMatrix->push_back(new vector <TYPE12> );
             for(int z = 0; z < matrix->at(i)->size(); z++)
             {
                newMatrix->back()->push_back(matrix->at(i)->at(z));
             }
         }*/

         newMatrix->erase(newMatrix->begin() + (row-1));

         return newMatrix;
    }

    template <typename TYPE13>
    vector <vector <TYPE13> * > * deleteColumn(int column, vector <vector <TYPE13> * > * matrix)
    {
         vector <vector <TYPE13> * > * newMatrix = this->copyMatrix(matrix);

         /*for(int i = 0; i < matrix->size(); i++)
         {
             newMatrix->push_back(new vector <TYPE13> );
             for(int z = 0; z < matrix->at(i)->size(); z++)
             {
                newMatrix->back()->push_back(matrix->at(i)->at(z));
             }
         }*/

         for(int size = 0; size < newMatrix->size(); size++)
            newMatrix->at(size)->erase(newMatrix->at(size)->begin() + (column-1));

         return newMatrix;
    }

    template <typename TYPE14>
    vector <vector <TYPE14> * > * copyMatrix(vector <vector <TYPE14> * > * matrix)
    {
         vector <vector <TYPE14> * > * newMatrix = new vector <vector <TYPE14> * >;

         for(int i = 0; i < matrix->size(); i++)
         {

             newMatrix->push_back(new vector <TYPE14> );

             for(int z = 0; z < matrix->at(i)->size(); z++)
             {
                newMatrix->back()->push_back(matrix->at(i)->at(z));
             }
         }

         return newMatrix;
    }

    template <typename TYPE15>
    vector <vector <TYPE15> * > * inverseMatrix(vector <vector <TYPE15> * > * matrix)
    {
         vector <vector <TYPE15> * > * newMatrix = new vector <vector <TYPE15> * >;
         vector <vector <TYPE15> * > * subdeterminantMatrix;

         double determinant = this->laplaceDeterminant(matrix);

         if(determinant == 0.0) return NULL; // non regular matrix

         for(int r = 1; r <= matrix->size(); r++)
         {
             newMatrix->push_back(new vector <TYPE15> );

             for(int c = 1; c <= matrix->back()->size(); c++)
             {
                subdeterminantMatrix = this->deleteRow(c, this->deleteColumn(r, matrix));


                // !!! INDEXES MUST BE IN REVERSE ORDER otherwise we get transposed inv. matrix
                double subdeterminant = this->laplaceDeterminant(subdeterminantMatrix);

                newMatrix->back()->push_back((pow((-1.0), r+c)*subdeterminant)/determinant);

                this->deleteMatrix(subdeterminantMatrix);
             }
         }

         return newMatrix;
    }

    template <typename TYPE16>
    vector <vector <TYPE16> * > * transposeMatrix(vector <vector <TYPE16> * > * matrix)
    {
         vector <vector <TYPE16> * > * newMatrix = new vector <vector <TYPE16> * >;

         for(int r = 1; r <= matrix->size(); r++)
         {
             newMatrix->push_back(new vector <TYPE16> );

             for(int c = 1; c <= matrix->back()->size(); c++)
             {
                   newMatrix->back()->push_back(this->getVal(c, r, matrix));
             }
         }

         return newMatrix;
    }

    /*------------------------------------BASIC MATRIX WORKAROUND END-------------------------------------------------*/
};

#endif // RADARUNIT_H
