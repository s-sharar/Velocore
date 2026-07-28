import { MongoClient } from "mongodb"
import { env } from "@/lib/env"

let cachedClient: MongoClient | null = null

export async function getDb() {
  if (!cachedClient) {
    cachedClient = await MongoClient.connect(env.mongodbUri)
  }
  const client = cachedClient
  const db = client.db(env.mongodbDb)
  return { client, db }
}



