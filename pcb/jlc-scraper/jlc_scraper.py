import requests, os
from flask import Flask, Response
from typing import Tuple, Optional, List, Dict
from bs4 import BeautifulSoup

def get_jlc_part_info(part_id: str) -> Tuple[Optional[int], List[Tuple[int, float]]]:
    """
    Fetches JLCPCB part information (stock quantity and price tiers) from the part detail page.

    Args:
        part_id: The JLCPCB part ID (e.g., 'C103174').

    Returns:
        A tuple containing:
        - The available stock quantity (int), or None if not found.
        - A list of price tiers, where each tier is a tuple (quantity, unit_price).
          Returns an empty list if price tiers are not found or cannot be parsed.

    Example:
       >> get_jlc_part_info("C103174")
       (1095, [(1, 0.0014), (1000, 0.0011), (3000, 0.0009), (10000, 0.0008), (50000, 0.0007)])
    """
    url = f"https://jlcpcb.com/partdetail/{part_id}"

    stock_qty: Optional[int] = None
    price_tiers: List[Tuple[int, float]] = []

    try:
        # Fetch the page content
        response = requests.get(url, timeout=10) # Added a timeout
        response.raise_for_status() # Raise an exception for bad status codes (4xx or 5xx)

        # Parse the HTML
        soup = BeautifulSoup(response.text, 'html.parser')

        # --- Extract Stock Quantity ---
        # Look for the div containing "In Stock: [number]"
        stock_div = soup.find('div', class_='text-16 font-bold', string=lambda text: text and 'In Stock:' in text)
        if stock_div:
            try:
                # Extract the number after "In Stock:"
                stock_text = stock_div.get_text(strip=True)
                stock_value_str = stock_text.split('In Stock:')[1].strip()
                stock_qty = int(stock_value_str)
            except (IndexError, ValueError):
                # Handle cases where the text format is unexpected
                print(f"Warning: Could not parse stock quantity from '{stock_div.get_text(strip=True)}' for {part_id}")
                stock_qty = None # Ensure it's None if parsing fails

        # --- Extract Price Tiers ---
        # Find the container for pricing information
        pricing_container = soup.find('div', class_='mt-10 bg-[#f8f8f8] py-10 px-30')

        if pricing_container:
            # Find all the rows within the pricing container
            # The rows have classes like 'flex items-center justify-between mt-14 cursor-pointer hover:text-2B8CED'
            price_rows = pricing_container.find_all('div', class_=lambda c: c and 'flex items-center justify-between' in c and 'mt-14' in c)

            for row in price_rows:
                spans = row.find_all('span', class_=lambda c: c and 'w-120' in c)
                if len(spans) >= 2:
                    try:
                        # Extract quantity (first span)
                        qty_text = spans[0].get_text(strip=True).replace('+', '').replace(',', '') # Remove '+' and ','
                        quantity = int(qty_text)

                        # Extract price (second span)
                        price_text = spans[1].get_text(strip=True).replace('$', '') # Remove '$'
                        unit_price = float(price_text)

                        price_tiers.append((quantity, unit_price))
                    except (ValueError, IndexError):
                        # Skip rows that don't have the expected format
                        print(f"Warning: Could not parse price tier from row: {row.get_text(strip=True)} for {part_id}")
                        continue # Move to the next row

    except requests.exceptions.RequestException as e:
        print(f"Error fetching data for {part_id}: {e}")
        # Return None and empty list on request failure
        return None, []
    except Exception as e:
        print(f"An unexpected error occurred while processing {part_id}: {e}")
        # Return None and empty list on other parsing failures
        return None, []

    return stock_qty, price_tiers


class JLCPartCache:
    """Caches results from get_jlc_part_info in memory."""
    def __init__(self):
        # The cache dictionary: part_id (str) -> (stock (Optional[int]), price_tiers (List[Tuple[int, float]]))
        self._cache: Dict[str, Tuple[Optional[int], List[Tuple[int, float]]]] = {}

    def get_info(self, part_id: str) -> Tuple[Optional[int], List[Tuple[int, float]]]:
        """
        Retrieves part info from cache if available, otherwise fetches using
        get_jlc_part_info and caches the result.
        """
        if part_id in self._cache:
            print(f"Cache hit for {part_id}") # Log cache hit
            return self._cache[part_id]
        else:
            print(f"Cache miss for {part_id}. Fetching...") # Log cache miss
            # Call the actual (or mocked) external function
            info = get_jlc_part_info(part_id)
            self._cache[part_id] = info
            return info

# Instantiate the cache globally when the application starts
part_cache = JLCPartCache()

app = Flask(__name__)

@app.route('/v1/part<part_id>/available', methods=['GET'])
def get_available_stock(part_id):
    """
    API endpoint to get available stock for a JLCPCB part.
    Returns stock quantity as text/csv, or -1 if not found.
    URL format: /v1/part{part_id}/available
    """
    stock, _ = part_cache.get_info(part_id)

    # Prepare the response text: stock quantity or -1 if None
    response_text = str(stock if stock is not None else -1)

    # Return the response with text/csv mime type
    return Response(response_text, mimetype='text/csv') # Status code defaults to 200 OK

@app.route('/v1/part<part_id>/price/qty<int:qty>', methods=['GET'])
def get_price(part_id, qty):
    """
    API endpoint to get the unit price for a JLCPCB part at a given quantity.
    Finds the best price tier (largest quantity <= requested qty).
    Returns the unit price as text/csv, or -1.0 if no price tiers found.
    URL format: /v1/part{part_id}/price/qty{qty}
    """
    _, price_tiers = part_cache.get_info(part_id)

    price = -1.0 # Default price if no tiers are found or if quantity is invalid

    if price_tiers:
        # Find the best tier: the one with the largest quantity <= requested qty
        # We iterate through tiers and keep track of the best match.
        # Since tiers are typically sorted by quantity ascending,
        # the last tier whose quantity is less than or equal to `qty` is the best one.
        best_unit_price = price_tiers[0][1] # Default to the price of the smallest tier
        found_matching_tier = False

        for tier_qty, unit_price in price_tiers:
            if qty >= tier_qty:
                best_unit_price = unit_price
                found_matching_tier = True
            # If tiers are sorted, we can stop early if tier_qty > qty
            # else:
            #     break # Optional optimization if sure about sorting

        # If no tier was found where qty >= tier_qty (i.e., qty is less than the smallest tier),
        # we return the price of the smallest tier (which best_unit_price was initialized to).
        # If there were tiers and at least one matched (qty >= tier_qty), best_unit_price
        # holds the price of the largest such tier.
        price = best_unit_price

    # Prepare the response text
    response_text = str(price)

    # Return the response with text/csv mime type
    return Response(response_text, mimetype='text/csv') # Status code defaults to 200 OK

# To run the Flask application directly from this script
if __name__ == '__main__':
    if 'DEBUG' in os.environ and os.environ['DEBUG'] == "1":
        # You can run the Flask development server using:
        # flask --app your_module_name run
        # Or directly using the app.run() method:
        # Note: debug=True is helpful during development for auto-reloading and error messages
        app.run(debug=True)
    else:
        print("Not in debug mode, exiting")

"""
if __name__ == "__main__":
    res = get_jlc_part_info("C103174")
    print(f"res = {res}")
    res = get_jlc_part_info("C1705")
    print(f"res = {res}")

"""

