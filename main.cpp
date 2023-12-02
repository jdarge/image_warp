#include <opencv2/opencv.hpp>
#include <vector>

const int MAX_DIM = 500;

std::vector<cv::Point> points;
cv::Mat homography;
cv::Mat warpedImage;

/*
0,0
MAX_DIM,0
MAX_DIM,MAX_DIM
0,MAX_DIM
*/
void reorderPoints() {
    // calc center of points
    cv::Point center(0, 0);
    for (const auto& point : points) {
        center.x += point.x;
        center.y += point.y;
    }
    center.x /= 4;
    center.y /= 4;

    // sort around center
    std::sort(points.begin(), points.end(), [center](const cv::Point& a, const cv::Point& b) {
        double angleA = atan2(a.y - center.y, a.x - center.x);
        double angleB = atan2(b.y - center.y, b.x - center.x);
        return angleA < angleB;
    });
}

void calculateHomography()
{

    std::vector<cv::Point2f> dstPoints;
    dstPoints.push_back(cv::Point2f(0, 0));
    dstPoints.push_back(cv::Point2f(MAX_DIM, 0));
    dstPoints.push_back(cv::Point2f(MAX_DIM, MAX_DIM));
    dstPoints.push_back(cv::Point2f(0, MAX_DIM));

    homography = cv::findHomography(points, dstPoints);
}

void onMouse(int event, int x, int y, int flags, void *param)
{
    if (event == cv::EVENT_LBUTTONDOWN && points.size() < 4)
    {
        points.push_back(cv::Point2f(x, y));

        if (points.size() == 4)
        {
            reorderPoints();
            calculateHomography();
        }
    }
}

void displayImage(cv::Mat &image)
{
    cv::Mat tempImage = image.clone();

    for (const auto &point : points)
    {
        cv::circle(tempImage, point, 5, cv::Scalar(0, 0, 255), -1); // Red circle
    }

    if (points.size() == 4)
    {
        cv::warpPerspective(image, warpedImage, homography, cv::Size(MAX_DIM, MAX_DIM));
        cv::imshow("Warped Image", warpedImage);
    }
    else
    {
        cv::imshow("Image", tempImage);
    }
}

// TODO preserve aspect ratio 
cv::Mat resizeImage(cv::Mat image)
{
    if (image.cols > MAX_DIM || image.rows > MAX_DIM)
    {
        cv::Size newSize;
        if (image.cols >= image.rows)
        {
            newSize = cv::Size(MAX_DIM, static_cast<int>((double)MAX_DIM / image.cols * image.rows));
        }
        else
        {
            newSize = cv::Size(static_cast<int>((double)MAX_DIM / image.rows * image.cols), MAX_DIM);
        }
        cv::resize(image, image, newSize);
    }

    return image;
}

int main()
{
    cv::Mat image = cv::imread("test.jpg");

    if (image.empty())
    {
        std::cerr << "Error: Could not open or find the image" << std::endl;
        return -1;
    }

    image = resizeImage(image);

    cv::namedWindow("Image");
    cv::setMouseCallback("Image", onMouse);

    while (true)
    {
        displayImage(image);

        int key = cv::waitKey(10);

        if (key == 'q' || key == 27)
        {
            break;
        }
    }

    cv::destroyAllWindows();

    // for (const auto& point : points) {
    //     std::cout << "Point: (" << point.x << ", " << point.y << ")" << std::endl;
    // }

    return 0;
}

