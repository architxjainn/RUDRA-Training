import sys
import cv2
import numpy as np

def main():
    # Attempt to read the target image file
    img_path = "img.png"
    src_img = cv2.imread(img_path)

    if src_img is None:
        sys.exit(f"Error: Could not find {img_path}")

    # Prepare visual canvas and convert color space to HSV
    canvas = src_img.copy()
    hsv_frame = cv2.cvtColor(src_img, cv2.COLOR_BGR2HSV)

    # Define lower and upper boundaries for hue wraparound in red spectrum
    red_bounds = [
        (np.array([0, 100, 100]), np.array([10, 255, 255])),
        (np.array([170, 100, 100]), np.array([180, 255, 255]))
    ]

    # Generate combined threshold binary mask for both color ranges
    combined_mask = cv2.bitwise_or(
        cv2.inRange(hsv_frame, *red_bounds[0]),
        cv2.inRange(hsv_frame, *red_bounds[1])
    )

    # Perform morphological opening followed by closing to clean up noise
    struct_elem = np.ones((5, 5), dtype=np.uint8)
    cleaned_mask = cv2.morphologyEx(combined_mask, cv2.MORPH_OPEN, struct_elem)
    cleaned_mask = cv2.morphologyEx(cleaned_mask, cv2.MORPH_CLOSE, struct_elem)

    # Retrieve external object boundaries
    found_contours, _ = cv2.findContours(
        cleaned_mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE
    )

    detected_rects = []

    # Iterate over contours to extract positional metadata and annotate image
    for cnt in found_contours:
        contour_area = cv2.contourArea(cnt)
        if contour_area <= 500:
            continue

        x_pos, y_pos, width, height = cv2.boundingRect(cnt)
        detected_rects.append((x_pos, y_pos, width, height))
        
        idx = len(detected_rects)
        mid_x = x_pos + (width // 2)
        mid_y = y_pos + (height // 2)

        # Draw green bounding outline around the detected region
        cv2.rectangle(
            canvas,
            pt1=(x_pos, y_pos),
            pt2=(x_pos + width, y_pos + height),
            color=(0, 255, 0),
            thickness=3
        )

        # Overlay identifying text label above the bounding box
        cv2.putText(
            canvas,
            text=f"Object {idx}",
            org=(x_pos, y_pos - 10),
            fontFace=cv2.FONT_HERSHEY_SIMPLEX,
            fontScale=0.7,
            color=(0, 255, 0),
            thickness=2
        )

        # Output detection details to the console
        print(
            f"Object {idx}: X={x_pos}, Y={y_pos}, "
            f"Width={width}, Height={height}, "
            f"Center=({mid_x}, {mid_y}), Area={contour_area}"
        )

    # Display total detection summary
    print("\n" + "=" * 30)
    print(f"Number of objects: {len(detected_rects)}")
    print("=" * 30)

    # Save processed result to disk
    cv2.imwrite("output.jpg", canvas)
    print("Output saved as output.jpg")

    # Display the final output in a GUI window until 'q' is pressed
    cv2.imshow("Detected Red Objects", canvas)
    
    while (cv2.waitKey(1) & 0xFF) != ord('q'):
        pass

    cv2.destroyAllWindows()

if __name__ == "__main__":
    main()
