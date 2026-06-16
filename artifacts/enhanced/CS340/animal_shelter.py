from pymongo import MongoClient
from pymongo.errors import PyMongoError


class AnimalShelter(object):
    """CRUD operations for the animals collection in the AAC MongoDB database."""

    def __init__(self, username: str, password: str) -> None:
        """
        Initialize the MongoDB client and connect to the AAC animals collection.
        """
        # Persist credentials and connection metadata for traceability
        self._username = username
        self._password = password

        self._host = "localhost"
        self._port = 27017
        self._database_name = "aac"
        self._collection_name = "animals"

        # Default to a disconnected state; subsequent operations must check connectivity
        self.client = None
        self.database = None
        self.collection = None

        # Basic sanity checks on constructor inputs to avoid unclear connection failures
        if not isinstance(username, str) or not username.strip():
            print("Initialization error: username must be a non-empty string.")
            return

        if not isinstance(password, str) or not password.strip():
            print("Initialization error: password must be a non-empty string.")
            return

        try:
            # Single responsibility: establish a connection using fixed project settings
            self.client = MongoClient(f"mongodb://{self._host}:{self._port}/")
            self.database = self.client[self._database_name]
            self.collection = self.database[self._collection_name]
        except PyMongoError as e:
            # Fail fast and keep the object in a clearly disconnected state
            print(f"Connection error: {e}")
            self.client = None
            self.database = None
            self.collection = None

    # ------------------------------ #
    # Internal validation helpers    #
    # ------------------------------ #

    def _validate_dict(self, value, allow_empty: bool = False) -> bool:
        """
        Validate that a value is a dictionary and optionally enforce non-emptiness.

        Centralizes dictionary validation so all CRUD operations apply the same rules.
        """
        if not isinstance(value, dict):
            return False
        if not allow_empty and not value:
            return False
        return True

    def _is_connected(self) -> bool:
        """
        Confirm that a collection reference is available before performing an operation.

        Avoids repeating connection checks in each CRUD method and ensures consistent behavior.
        """
        if self.collection is None:
            print("Operation skipped: database connection is not available.")
            return False
        return True

    # ------------------------------ #
    # Create                         #
    # ------------------------------ #

    def create(self, data: dict) -> bool:
        """
        Insert a document into the animals collection.

        :param data: Dictionary of key/value pairs for the new document.
        :return: True if insert succeeds, otherwise False.
        """
        if not self._is_connected():
            return False

        if not self._validate_dict(data, allow_empty=False):
            print("Create aborted: data must be a non-empty dictionary.")
            return False

        try:
            result = self.collection.insert_one(data)
            return bool(result.acknowledged)
        except PyMongoError as e:
            print(f"Insert error: {e}")
            return False

    # ------------------------------ #
    # Read                           #
    # ------------------------------ #

    def read(self, query: dict | None) -> list:
        """
        Query for documents using the find() API.

        :param query: Dictionary used as filter for find(). If None, all documents
                      are returned.
        :return: List of matching documents; empty list if none or on error.
        """
        if not self._is_connected():
            return []

        # None signifies a request for all documents in the collection
        if query is None:
            query = {}

        if not self._validate_dict(query, allow_empty=True):
            print("Read aborted: query must be a dictionary.")
            return []

        try:
            cursor = self.collection.find(query)
            return list(cursor)
        except PyMongoError as e:
            print(f"Read error: {e}")
            return []

    # ------------------------------ #
    # Update                         #
    # ------------------------------ #

    def update(self, query: dict, new_values: dict) -> int:
        """
        Update document(s) in the animals collection.

        :param query: Dictionary filter to locate document(s).
        :param new_values: Dictionary of key/value pairs with the updated data.
        :return: Integer count of documents modified; 0 on error or no match.
        """
        if not self._is_connected():
            return 0

        if not self._validate_dict(query, allow_empty=False):
            print("Update aborted: query must be a non-empty dictionary.")
            return 0

        if not self._validate_dict(new_values, allow_empty=False):
            print("Update aborted: new_values must be a non-empty dictionary.")
            return 0

        try:
            # Explicit use of $set minimizes the risk of unintended document replacement
            result = self.collection.update_many(query, {"$set": new_values})
            return int(result.modified_count)
        except PyMongoError as e:
            print(f"Update error: {e}")
            return 0

    # ------------------------------ #
    # Delete                         #
    # ------------------------------ #

    def delete(self, query: dict) -> int:
        """
        Remove document(s) from the animals collection.

        :param query: Dictionary filter to locate document(s).
        :return: Integer count of documents deleted; 0 on error or no match.
        """
        if not self._is_connected():
            return 0

        # Explicitly require a non-empty filter to reduce risk of mass deletion
        if not self._validate_dict(query, allow_empty=False):
            print("Delete aborted: query must be a non-empty dictionary.")
            return 0

        try:
            result = self.collection.delete_many(query)
            return int(result.deleted_count)
        except PyMongoError as e:
            print(f"Delete error: {e}")
            return 0