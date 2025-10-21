import os
import logging
import traceback


logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger(__name__)

def predict_from_db():
    try:
        
        import sqlite3
        import pandas as pd
        import numpy as np
        import tensorflow as tf
        
        from tensorflow.keras import layers, models
        from sklearn.preprocessing import MinMaxScaler
        
        
        logger.info(f"Using TensorFlow version: {tf.__version__}")
        
        logger.info("Libraries imported successfully")
        
        
        model_path = "bilstm_gru_aqi_model.h5"
        if not os.path.exists(model_path):
            logger.error(f"Model file not found at {os.path.abspath(model_path)}")
            return {"error": f"Model file not found at {os.path.abspath(model_path)}"}
        
        logger.info("Loading model...")
        
        model = models.load_model(model_path, compile=False)
        logger.info("Model loaded successfully")
        
        
        db_path = "aqi_data.db"
        if not os.path.exists(db_path):
            logger.error(f"Database file not found at {os.path.abspath(db_path)}")
            return {"error": f"Database file not found at {os.path.abspath(db_path)}"}
        
        logger.info("Connecting to database...")
        conn = sqlite3.connect(db_path)
        
        
        try:
            test_query = "SELECT name FROM sqlite_master WHERE type='table' AND name='aqi_readings';"
            table_check = pd.read_sql_query(test_query, conn)
            if len(table_check) == 0:
                logger.error("Table 'aqi_readings' not found in database")
                return {"error": "Table 'aqi_readings' not found in database"}
        except Exception as e:
            logger.error(f"Error checking database tables: {e}")
            return {"error": f"Database error: {str(e)}"}
        
        logger.info("Reading data from database...")
        query = """
            SELECT NO, NO2, NOx, NH3, CO, Benzene, Toluene, Xylene, AQI 
            FROM aqi_readings ORDER BY id DESC LIMIT 24
        """
        
        try:
            df = pd.read_sql_query(query, conn).iloc[::-1]
        except Exception as e:
            logger.error(f"Error in SQL query: {e}")
            return {"error": f"SQL query error: {str(e)}"}
        finally:
            conn.close()
        
        logger.info(f"Retrieved {len(df)} rows from database")
        
        if len(df) != 24:
            logger.warning(f"Only {len(df)} rows retrieved, need 24 rows")
            return {"error": f"Need exactly 24 rows in the database. Found: {len(df)}"}
        
        logger.info("Preprocessing data...")
        scaler = MinMaxScaler()
        scaler.fit(df)
        scaled = scaler.transform(df)
        
        logger.info("Making prediction...")
        input_seq = np.expand_dims(scaled, axis=0)
        
        pred_scaled = model.predict(input_seq, verbose=0)
        pred_inverse = scaler.inverse_transform(pred_scaled)
        
        predicted_aqi = round(float(pred_inverse[0][-1]), 2)
        logger.info(f"Prediction successful: AQI = {predicted_aqi}")
        
        return {"predicted_aqi": predicted_aqi}

    except Exception as e:
        logger.error(f"Unexpected error: {e}")
        logger.error(traceback.format_exc())
        return {"error": str(e), "traceback": traceback.format_exc()}


if __name__ == "__main__":
    print("Testing prediction function...")
    result = predict_from_db()
    print(f"Result: {result}")