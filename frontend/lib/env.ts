export const env = {
  mongodbUri: process.env.MONGODB_URI || "mongodb://127.0.0.1:27017",
  mongodbDb: process.env.MONGODB_DB || "velocore",
  jwtSecret: process.env.JWT_SECRET || "dev-secret-change-me",
}



