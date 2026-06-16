# Example Python Code to Insert a Document 

from pymongo import MongoClient 
from bson.objectid import ObjectId 
from pymongo.errors import PyMongoError


class AnimalShelter(object): 
    """ CRUD operations for Animal collection in MongoDB """ 

    def __init__(self, username, password): 
        # Initializing the MongoClient. This helps to access the MongoDB 
        # databases and collections. This version uses dynamic login credentials 
        # passed into the constructor, while keeping the database and 
        # collection names fixed for this project. 
        # 
        # Connection Variables 
        USER = username 
        PASS = password      
        HOST = 'localhost' 
        PORT = 27017 
        DB = 'aac' 
        COL = 'animals' 

        # Initialize Connection 
        try:
            self.client = MongoClient(f"mongodb://{HOST}:{PORT}/")
            self.database = self.client[DB]
            self.collection = self.database[COL]
        except PyMongoError as e:
            print(f"Connection error: {e}")
            self.client = None
            self.database = None
            self.collection = None
    #--------------------------------#
    # Create                         #
    #--------------------------------#
    
    def create(self, data):
        """
        Insert a document into the animals collection.

        :param data: Dictionary of key/value pairs for the new document.
        :return: True if insert succeeds, otherwise False.
        """
        if self.collection is None:
            return False

        if data is None or not isinstance(data, dict) or not data:
            # Nothing to save or invalid data type
            return False

        try:
            result = self.collection.insert_one(data)  # data should be dictionary
            return result.acknowledged
        except PyMongoError as e:
            print(f"Insert error: {e}")
            return False

    #------------------------------#
    # Read                         #
    #------------------------------#
    
    def read(self, query):
        """
        Query for documents using the find() API.

        :param query: Dictionary used as filter for find().
        :return: List of matching documents; empty list if none or on error.
        """
        if self.collection is None:
            return []

        if query is None:
            query = {}

        if not isinstance(query, dict):
            return []

        try:
            cursor = self.collection.find(query)  
            return list(cursor)
        except PyMongoError as e:
            print(f"Read error: {e}")
            return []
    
    # ------------------------------------------------------------------ #
    # UPDATE                                                             #
    # ------------------------------------------------------------------ #
    
    def update(self, query, new_values):
        """
        Query for and update document(s) in the animals collection.

        :param query:      Dictionary key/value filter to locate document(s).
        :param new_values: Dictionary of key/value pairs with the updated data.
        :return: Integer count of documents modified; 0 on error or no match.
        """
        if self.collection is None:
            return 0

        # Validate both arguments are non-empty dictionaries
        if not isinstance(query, dict) or not query:
            return 0
        if not isinstance(new_values, dict) or not new_values:
            return 0

        try:
            # update_many() applies the change to every matching document
            result = self.collection.update_many(query, {"$set": new_values})
            return result.modified_count
        except PyMongoError as e:
            print(f"Update error: {e}")
            return 0
    
    # ------------------------------------------------------------------ #
    # DELETE                                                             #
    # ------------------------------------------------------------------ #
   
    def delete(self, query):
        """
        Query for and remove document(s) from the animals collection.

        :param query: Dictionary key/value filter to locate document(s).
        :return: Integer count of documents deleted; 0 on error or no match.
        """
        if self.collection is None:
            return 0

        # Validate input: must be a non-empty dictionary to prevent mass deletion
        if not isinstance(query, dict) or not query:
            return 0

        try:
            # delete_many() removes every document that matches the filter
            result = self.collection.delete_many(query)
            return result.deleted_count
        except PyMongoError as e:
            print(f"Delete error: {e}")
            return 0